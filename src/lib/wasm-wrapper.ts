import type { OsuFileFormat } from "../types/types";

let wasm_instance: any = null;
let init_promise: Promise<void> | null = null;
let wasm_factory: ((module_config?: Record<string, unknown>) => any) | null = null;
let wasm_global_name = "create_osu_parser";
const script_loaders: Map<string, Promise<void>> = new Map();

type wasm_init_options = {
    factory?: (module_config?: Record<string, unknown>) => any;
    script_url?: string;
    global_name?: string;
    module_config?: Record<string, unknown>;
};

const get_global_name = (options: wasm_init_options): string => {
    return options.global_name || wasm_global_name;
};

const load_script = (script_url: string): Promise<void> => {
    const existing = script_loaders.get(script_url);
    if (existing) return existing;

    const loader = new Promise<void>((resolve, reject) => {
        if (typeof document == "undefined") {
            reject(new Error("script loading requires a browser environment"));
            return;
        }

        const script = document.createElement("script");
        script.src = script_url;
        script.async = true;

        script.onload = () => resolve();
        script.onerror = () => reject(new Error(`failed to load wasm script: ${script_url}`));

        document.head.appendChild(script);
    });

    script_loaders.set(script_url, loader);
    return loader;
};

const resolve_factory = async (options: wasm_init_options): Promise<any> => {
    if (options.factory) return options.factory;
    if (wasm_factory) return wasm_factory;

    if (options.script_url) {
        await load_script(options.script_url);
    }

    const global_name = get_global_name(options);
    return (globalThis as any)[global_name];
};

export const set_wasm_factory = (
    factory: (module_config?: Record<string, unknown>) => any,
    global_name?: string
): void => {
    wasm_factory = factory;
    if (global_name) {
        wasm_global_name = global_name;
    }
};

export const is_wasm_ready = (): boolean => {
    return wasm_instance != null;
};

export const init_wasm = async (options: wasm_init_options = {}): Promise<void> => {
    if (wasm_instance) return;
    if (init_promise) return init_promise;

    init_promise = (async () => {
        const factory = await resolve_factory(options);
        if (typeof factory != "function") {
            const global_name = get_global_name(options);
            throw new Error(`wasm factory not found: ${global_name}`);
        }

        wasm_instance = await factory(options.module_config);
    })();

    return init_promise.catch((err) => {
        init_promise = null;
        throw err;
    });
};

export const init_wasm_from_url = async (
    script_url: string,
    module_config: Record<string, unknown> = {},
    global_name?: string
): Promise<void> => {
    return init_wasm({ script_url, module_config, global_name });
};

export const get_property = (data: Uint8Array, key: string): string => {
    if (wasm_instance == null) throw new Error("wasm not initialized");

    const buffer_ptr = wasm_instance._malloc(data.length);
    wasm_instance.HEAPU8.set(data, buffer_ptr);

    try {
        return wasm_instance.get_property(buffer_ptr, data.length, key);
    } finally {
        wasm_instance._free(buffer_ptr);
    }
};

export const get_properties = (
    data: Uint8Array,
    keys: string[]
): Record<string, string> => {
    if (wasm_instance == null) throw new Error("wasm not initialized");

    const buffer_ptr = wasm_instance._malloc(data.length);
    wasm_instance.HEAPU8.set(data, buffer_ptr);

    try {
        return wasm_instance.get_properties(buffer_ptr, data.length, keys);
    } finally {
        wasm_instance._free(buffer_ptr);
    }
};

export const get_section = (data: Uint8Array, section: string): string[] => {
    if (wasm_instance == null) throw new Error("wasm not initialized");

    const buffer_ptr = wasm_instance._malloc(data.length);
    wasm_instance.HEAPU8.set(data, buffer_ptr);

    try {
        return wasm_instance.get_section(buffer_ptr, data.length, section);
    } finally {
        wasm_instance._free(buffer_ptr);
    }
};

export const parse = (data: Uint8Array): OsuFileFormat => {
    if (wasm_instance == null) throw new Error("wasm not initialized");

    const buffer_ptr = wasm_instance._malloc(data.length);
    wasm_instance.HEAPU8.set(data, buffer_ptr);

    try {
        return wasm_instance.parse(buffer_ptr, data.length);
    } finally {
        wasm_instance._free(buffer_ptr);
    }
};
