import type { OsuFileFormat } from "../types/types";

export declare const is_wasm_ready: () => boolean;
export declare const init_wasm: () => Promise<void>;
export declare const get_property: (data: Uint8Array, key: string) => Promise<string>;
export declare const get_properties: (data: Uint8Array, keys: string[]) => Promise<Record<string, string>>;
export declare const get_section: (data: Uint8Array, section: string) => Promise<string[]>;
export declare const parse: (data: Uint8Array) => Promise<OsuFileFormat>;

declare const api: {
    init_wasm: () => Promise<void>;
    is_wasm_ready: () => boolean;
    get_property: (data: Uint8Array, key: string) => Promise<string>;
    get_properties: (data: Uint8Array, keys: string[]) => Promise<Record<string, string>>;
    get_section: (data: Uint8Array, section: string) => Promise<string[]>;
    parse: (data: Uint8Array) => Promise<OsuFileFormat>;
};

export default api;
