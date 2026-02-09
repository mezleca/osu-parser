import { OsuKey, OsuInput, OsuFileFormat } from "./types/types";
import { native } from "./lib/bindings";

export const get_property = (data: Uint8Array, key: OsuKey): string => {
    return native.get_property(data, key);
};

export const get_properties = (input: Uint8Array | OsuInput, keys: OsuKey[]): Record<string, string> => {
    const data = input instanceof Uint8Array ? input : input.data;
    const result = native.get_properties(data, keys);

    if (!(input instanceof Uint8Array) && input.id) {
        return { ...result, id: input.id };
    }

    return result;
};

export const get_section = (data: Uint8Array, section: string): string[] => {
    return native.get_section(data, section);
};

export const parse = (input: Uint8Array | OsuInput): OsuFileFormat => {
    const data = input instanceof Uint8Array ? input : input.data;
    return native.parse(data);
};

const parser = {
    get_property,
    get_properties,
    get_section,
    parse
};

export type { OsuKey, OsuInput, OsuFileFormat };
export * from "./types/types";
export default parser;
