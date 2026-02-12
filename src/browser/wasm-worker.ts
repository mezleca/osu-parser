let wasm_instance: any = null;

type worker_message = {
    id: number;
    type: "init" | "call";
    payload: {
        script_url?: string;
        global_name?: string;
        module_config?: Record<string, unknown>;
        method_name?: string;
        data?: ArrayBuffer;
        args?: unknown[];
    };
};

const parse_error = (error: unknown): string => {
    if (error instanceof Error) {
        return error.message;
    }

    return String(error);
};

const reply_ok = (id: number, result: unknown): void => {
    self.postMessage({ id, ok: true, result });
};

const reply_error = (id: number, error: unknown): void => {
    self.postMessage({ id, ok: false, error: parse_error(error) });
};

const run_wasm_call = (method_name: string, data: Uint8Array, args: unknown[]): unknown => {
    if (wasm_instance == null) {
        throw new Error("wasm not initialized");
    }

    const method = wasm_instance[method_name];
    if (typeof method != "function") {
        throw new Error(`unknown wasm method: ${method_name}`);
    }

    const buffer_ptr = wasm_instance._malloc(data.length);
    wasm_instance.HEAPU8.set(data, buffer_ptr);

    try {
        return method(buffer_ptr, data.length, ...args);
    } finally {
        wasm_instance._free(buffer_ptr);
    }
};

const resolve_factory_from_script = async (script_url: string, global_name: string): Promise<any> => {
    const response = await fetch(script_url);
    if (!response.ok) {
        throw new Error(`failed to load wasm script: ${script_url}`);
    }

    const source = (await response.text()).replace(/\n\/\/# sourceMappingURL=.*$/g, "");
    const evaluator = new Function(
        "self",
        "global_name",
        `${source}\n; return self[global_name] ?? (typeof create_osu_parser != "undefined" ? create_osu_parser : null);`
    );

    return evaluator(self, global_name);
};

self.onmessage = async (event: MessageEvent<worker_message>) => {
    const message = event.data;
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

            const factory = await resolve_factory_from_script(script_url, global_name);

            if (typeof factory != "function") {
                throw new Error(`wasm factory not found in worker: ${global_name}`);
            }

            wasm_instance = await factory(module_config);
            reply_ok(id, true);
            return;
        }

        if (type == "call") {
            const method_name = payload.method_name;
            const raw_data = payload.data;
            const args = payload.args || [];

            if (!method_name || typeof method_name != "string") {
                throw new Error("missing method_name for worker call");
            }

            if (!raw_data) {
                throw new Error("missing data for worker call");
            }

            const data = new Uint8Array(raw_data);
            const result = run_wasm_call(method_name, data, args);
            reply_ok(id, result);
            return;
        }

        throw new Error(`unknown worker message type: ${type}`);
    } catch (error) {
        reply_error(id, error);
    }
};
