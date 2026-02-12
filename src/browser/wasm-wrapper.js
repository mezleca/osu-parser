let wasm_worker = null;
let worker_init_promise = null;
let worker_request_id = 0;
let worker_is_ready = false;
const worker_pending_requests = new Map();
const resolve_wasm_factory_url = () => {
    const resolved = new URL("./osu-beatmap-parser.js", import.meta.url).toString();

    if (resolved.includes("/@fs/") || resolved.includes("/node_modules/.vite/deps/")) {
        return new URL("/osu-beatmap-parser.js", globalThis.location.href).toString();
    }

    return resolved;
};

const wasm_factory_url = resolve_wasm_factory_url();

const worker_script = `
let wasm_instance = null;

const parse_error = (error) => {
    if (error && typeof error.message == "string") {
        return error.message;
    }

    return String(error);
};

const reply_ok = (id, result) => {
    self.postMessage({ id, ok: true, result });
};

const reply_error = (id, error) => {
    self.postMessage({ id, ok: false, error: parse_error(error) });
};

const run_wasm_call = (method_name, data, args) => {
    if (wasm_instance == null) {
        throw new Error("wasm not initialized");
    }

    const method = wasm_instance[method_name];
    if (typeof method != "function") {
        throw new Error(\`unknown wasm method: \${method_name}\`);
    }

    const buffer_ptr = wasm_instance._malloc(data.length);
    wasm_instance.HEAPU8.set(data, buffer_ptr);

    try {
        return method(buffer_ptr, data.length, ...args);
    } finally {
        wasm_instance._free(buffer_ptr);
    }
};

self.onmessage = async (event) => {
    const message = event.data || {};
    const id = message.id;
    const type = message.type;
    const payload = message.payload || {};

    try {
        if (type == "init") {
            const script_url = payload.script_url;
            const global_name = payload.global_name || "create_osu_parser";
            const module_config = payload.module_config || {};

            if (!script_url) {
                throw new Error("worker mode requires script_url");
            }

            self.importScripts(script_url);

            const factory = self[global_name];
            if (typeof factory != "function") {
                throw new Error(\`wasm factory not found in worker: \${global_name}\`);
            }

            wasm_instance = await factory(module_config);
            reply_ok(id, true);
            return;
        }

        if (type == "call") {
            const method_name = payload.method_name;
            const args = payload.args || [];
            const raw_data = payload.data;
            const data = raw_data ? new Uint8Array(raw_data) : null;
            if (data == null) {
                throw new Error("missing data for worker call");
            }

            const result = run_wasm_call(method_name, data, args);
            reply_ok(id, result);
            return;
        }

        throw new Error(\`unknown worker message type: \${type}\`);
    } catch (error) {
        reply_error(id, error);
    }
};
`;

const setup_worker_listener = () => {
    if (wasm_worker == null) return;

    wasm_worker.onmessage = (event) => {
        const data = event.data || {};
        const id = data.id;
        const ok = data.ok;
        const result = data.result;
        const error = data.error;

        if (typeof id != "number") {
            return;
        }

        const handlers = worker_pending_requests.get(id);
        if (!handlers) {
            return;
        }

        worker_pending_requests.delete(id);

        if (ok) {
            handlers.resolve(result);
            return;
        }

        handlers.reject(new Error(error || "worker request failed"));
    };

    wasm_worker.onerror = (event) => {
        const message = event.message || "worker crashed";
        worker_is_ready = false;
        for (const [, handlers] of worker_pending_requests) {
            handlers.reject(new Error(message));
        }
        worker_pending_requests.clear();
    };
};

const worker_request = (type, payload, transfer_list = []) => {
    if (wasm_worker == null) {
        return Promise.reject(new Error("wasm worker not initialized"));
    }

    worker_request_id += 1;
    const id = worker_request_id;

    return new Promise((resolve, reject) => {
        worker_pending_requests.set(id, { resolve, reject });
        wasm_worker?.postMessage({ id, type, payload }, transfer_list);
    });
};

const copy_transferable_data = (data) => {
    return data.slice().buffer;
};

export const is_wasm_ready = () => {
    return wasm_worker != null && worker_is_ready;
};

export const init_wasm = async () => {
    if (worker_is_ready) {
        return;
    }

    if (worker_init_promise) {
        return worker_init_promise;
    }

    worker_init_promise = (async () => {
        if (typeof Worker == "undefined" || typeof Blob == "undefined" || typeof URL == "undefined") {
            throw new Error("wasm wrapper requires browser Worker + Blob + URL support");
        }

        const blob = new Blob([worker_script], { type: "application/javascript" });
        const worker_url = URL.createObjectURL(blob);

        try {
            wasm_worker = new Worker(worker_url);
            setup_worker_listener();

            await worker_request("init", {
                script_url: wasm_factory_url,
                global_name: "create_osu_parser",
                module_config: {},
            });
            worker_is_ready = true;
        } finally {
            URL.revokeObjectURL(worker_url);
        }
    })();

    return worker_init_promise.catch((err) => {
        worker_init_promise = null;
        wasm_worker?.terminate();
        wasm_worker = null;
        worker_is_ready = false;
        throw err;
    });
};

export const get_property = async (data, key) => {
    const worker_data = copy_transferable_data(data);
    const result = await worker_request("call", { method_name: "get_property", data: worker_data, args: [key] }, [worker_data]);
    return result;
};

export const get_properties = async (data, keys) => {
    const worker_data = copy_transferable_data(data);
    const result = await worker_request("call", { method_name: "get_properties", data: worker_data, args: [keys] }, [worker_data]);
    return result;
};

export const get_section = async (data, section) => {
    const worker_data = copy_transferable_data(data);
    const result = await worker_request("call", { method_name: "get_section", data: worker_data, args: [section] }, [worker_data]);
    return result;
};

export const parse = async (data) => {
    const worker_data = copy_transferable_data(data);
    const result = await worker_request("call", { method_name: "parse", data: worker_data, args: [] }, [worker_data]);
    return result;
};

const api = {
    init_wasm,
    is_wasm_ready,
    get_property,
    get_properties,
    get_section,
    parse,
};

export default api;
