import {
    init_wasm,
    get_property,
    get_properties,
    get_section,
    parse,
    set_wasm_factory,
    is_wasm_ready,
    init_wasm_from_url,
} from "./wasm-wrapper";

const api = {
    init_wasm,
    get_property,
    get_properties,
    get_section,
    parse,
    set_wasm_factory,
    is_wasm_ready,
    init_wasm_from_url,
};

(globalThis as any).beatmap_parser = api;

export {
    init_wasm,
    get_property,
    get_properties,
    get_section,
    parse,
    set_wasm_factory,
    is_wasm_ready,
    init_wasm_from_url,
};
