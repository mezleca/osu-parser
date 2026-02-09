import { describe, expect, test } from "bun:test";
import fs from "fs";
import path from "path";

type parser_api = {
    get_property: (data: Uint8Array, key: string) => string;
    get_properties: (data: Uint8Array, keys: string[]) => Record<string, string>;
    get_section: (data: Uint8Array, section: string) => string[];
    parse: (data: Uint8Array) => any;
};

const DATA_DIR = path.join(import.meta.dir, "data");
const DATA_FILES = ["1.osu", "2.osu", "jeff.osu"];

const load_data = (name: string): Uint8Array => {
    const file_path = path.join(DATA_DIR, name);
    const buf = fs.readFileSync(file_path);
    return new Uint8Array(buf);
};

const run_common_tests = (label: string, api: parser_api) => {
    describe(label, () => {
        test("get_property returns metadata", () => {
            const data = load_data("1.osu");

            const title = api.get_property(data, "Title");
            const artist = api.get_property(data, "Artist");

            expect(title).toBe("Pinball");
            expect(artist).toBe("Nightcore");
        });

        test("parse returns expected sections", () => {
            const data = load_data("1.osu");
            const parsed = api.parse(data);

            expect(parsed.version).toBe(6);
            expect(parsed.Metadata.Creator).toBe("jericho2442");
            expect(parsed.General.Mode).toBe(0);
            expect(parsed.Difficulty.SliderTickRate).toBe(2);
            expect(parsed.Events.background?.filename).toBe("nightcore III.jpg");
        });

        test("slider defaults are normalized", () => {
            const data = load_data("1.osu");
            const parsed = api.parse(data);
            const slider = parsed.HitObjects.find((obj: any) => (obj.type & 2) != 0);

            if (!slider) return;

            expect(slider.edgeSounds.length).toBe(slider.slides + 1);
            expect(slider.edgeSets.length).toBe(slider.slides + 1);
            expect(slider.hitSample).toBeTruthy();
        });

        test("timing points preserve red/green ordering at same time", () => {
            const data = load_data("jeff.osu");
            const parsed = api.parse(data);
            const points = parsed.TimingPoints;
            const at_225 = points.filter((tp: any) => tp.time === 225);

            expect(at_225.length).toBeGreaterThanOrEqual(2);
            expect(at_225[0].beatLength).toBeCloseTo(666.666666666667, 6);
            expect(at_225[0].uninherited).toBe(1);
            expect(at_225[1].beatLength).toBe(-200);
            expect(at_225[1].uninherited).toBe(0);
        });

        for (const name of DATA_FILES) {
            test(`parse works for ${name}`, () => {
                const data = load_data(name);
                const parsed = api.parse(data);
                expect(parsed).toBeTruthy();
            });
        }
    });
};

const load_native_api = async (): Promise<parser_api | null> => {
    try {
        const mod = await import("../src/index");
        return mod.default ?? mod;
    } catch {
        return null;
    }
};

const load_wasm_api = async (): Promise<parser_api | null> => {
    const wasm_js = path.join(import.meta.dir, "..", "build", "osu-beatmap-parser.js");
    if (!fs.existsSync(wasm_js)) return null;

    const source_files = [
        path.join(import.meta.dir, "..", "src", "native", "osu", "parser.cpp"),
        path.join(import.meta.dir, "..", "src", "native", "wasm.cpp"),
        path.join(import.meta.dir, "..", "src", "lib", "wasm-wrapper.ts"),
        path.join(import.meta.dir, "..", "src", "lib", "wasm-browser.ts"),
        path.join(import.meta.dir, "..", "scripts", "build.ts")
    ];

    const wasm_mtime = fs.statSync(wasm_js).mtimeMs;
    const latest_source_mtime = Math.max(
        ...source_files.filter((file_path) => fs.existsSync(file_path)).map((file_path) => fs.statSync(file_path).mtimeMs)
    );

    if (wasm_mtime < latest_source_mtime) return null;

    const wasm_mod = await import(wasm_js);
    const factory = wasm_mod.default ?? wasm_mod.create_osu_parser ?? wasm_mod;

    if (typeof factory != "function") return null;

    const wrapper = await import("../src/lib/wasm-wrapper");

    wrapper.set_wasm_factory(factory);
    await wrapper.init_wasm();

    return {
        get_property: wrapper.get_property,
        get_properties: wrapper.get_properties,
        get_section: wrapper.get_section,
        parse: wrapper.parse
    };
};

const native_api = await load_native_api();
const wasm_api = await load_wasm_api();

if (native_api) {
    run_common_tests("native", native_api);
} else {
    test.skip("native", () => {});
}

if (wasm_api) {
    run_common_tests("wasm", wasm_api);
} else {
    test.skip("wasm", () => {});
}
