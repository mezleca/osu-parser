import type { OsuFileFormat } from "../types/types";

type worker_pending = {
    resolve: (value: any) => void;
    reject: (error: Error) => void;
};

let wasm_worker: Worker | null = null;
let worker_init_promise: Promise<void> | null = null;
let worker_request_id = 0;
let worker_is_ready = false;

const worker_pending_requests: Map<number, worker_pending> = new Map();
const wasm_factory_url = new URL("./osu-beatmap-parser.js", import.meta.url).toString();
const wasm_worker_url = new URL("./wasm-worker.js", import.meta.url).toString();

const setup_worker_listener = (): void => {
    if (wasm_worker == null) {
        return;
    }

    wasm_worker.onmessage = (event: MessageEvent<any>) => {
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

    wasm_worker.onerror = (event: ErrorEvent) => {
        const message = event.message || "worker crashed";
        worker_is_ready = false;

        for (const [, handlers] of worker_pending_requests) {
            handlers.reject(new Error(message));
        }
        worker_pending_requests.clear();
    };
};

const worker_request = (type: "init" | "call", payload: Record<string, unknown>, transfer_list: Transferable[] = []): Promise<any> => {
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

const copy_transferable_data = (data: Uint8Array): ArrayBuffer => {
    return data.slice().buffer;
};

export const is_wasm_ready = (): boolean => {
    return wasm_worker != null && worker_is_ready;
};

export const init_wasm = async (): Promise<void> => {
    if (worker_is_ready) {
        return;
    }

    if (worker_init_promise) {
        return worker_init_promise;
    }

    worker_init_promise = (async () => {
        if (typeof Worker == "undefined") {
            throw new Error("wasm wrapper requires browser Worker support");
        }

        wasm_worker = new Worker(wasm_worker_url, { type: "module" });
        setup_worker_listener();

        await worker_request("init", {
            script_url: wasm_factory_url,
            global_name: "create_osu_parser",
            module_config: {},
        });

        worker_is_ready = true;
    })();

    return worker_init_promise.catch((err) => {
        worker_init_promise = null;
        wasm_worker?.terminate();
        wasm_worker = null;
        worker_is_ready = false;
        throw err;
    });
};

export const get_property = async (data: Uint8Array, key: string): Promise<string> => {
    const worker_data = copy_transferable_data(data);
    const result = await worker_request("call", { method_name: "get_property", data: worker_data, args: [key] }, [worker_data]);
    return result;
};

export const get_properties = async (data: Uint8Array, keys: string[]): Promise<Record<string, string>> => {
    const worker_data = copy_transferable_data(data);
    const result = await worker_request("call", { method_name: "get_properties", data: worker_data, args: [keys] }, [worker_data]);
    return result;
};

export const get_section = async (data: Uint8Array, section: string): Promise<string[]> => {
    const worker_data = copy_transferable_data(data);
    const result = await worker_request("call", { method_name: "get_section", data: worker_data, args: [section] }, [worker_data]);
    return result;
};

export const parse = async (data: Uint8Array): Promise<OsuFileFormat> => {
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
