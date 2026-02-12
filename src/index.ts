import { OsuKey, OsuInput, OsuFileFormat } from "./types/types";
import { native } from "./lib/bindings";

export const get_property = async (data: Uint8Array, key: OsuKey): Promise<string> => {
    return await native.get_property(data, key);
};

export const get_properties = async (input: Uint8Array | OsuInput, keys: OsuKey[]): Promise<Record<string, string>> => {
    const data = input instanceof Uint8Array ? input : input.data;
    const result = await native.get_properties(data, keys);

    if (!(input instanceof Uint8Array) && input.id) {
        return { ...result, id: input.id };
    }

    return result;
};

export const get_section = async (data: Uint8Array, section: string): Promise<string[]> => {
    return await native.get_section(data, section);
};

export const parse = async (input: Uint8Array | OsuInput): Promise<OsuFileFormat> => {
    const data = input instanceof Uint8Array ? input : input.data;
    return await native.parse(data);
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
