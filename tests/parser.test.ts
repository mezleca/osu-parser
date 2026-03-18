import { describe, expect, test } from "bun:test";
import fs from "fs";
import os from "os";
import path from "path";

import { OsdbParser } from "../dist/index";

const ROOT = path.join(import.meta.dir, "data");

const files = {
    osdb: "collections/mathi.osdb"
};

const withTempCopy = (source: string) => {
    const temp_dir = fs.mkdtempSync(path.join(os.tmpdir(), "osu-parser-"));
    const temp_path = path.join(temp_dir, path.basename(source));
    fs.copyFileSync(source, temp_path);
    return temp_path;
};

const roundtrip = <T>(
    create: () => { parse: (location: string) => unknown; get: () => T; write: () => void; free: () => void },
    source: string,
    assert: (first: T, second: T) => void
) => {
    const temp_path = withTempCopy(source);

    const parser = create();
    parser.parse(temp_path);
    const first = parser.get();
    parser.write();
    parser.free();

    const parser2 = create();
    parser2.parse(temp_path);
    const second = parser2.get();
    parser2.free();

    assert(first, second);
};

describe("osdb parser", () => {
    test("parse file", () => {
        const parser = new OsdbParser();
        const file_path = path.join(ROOT, files.osdb);

        parser.parse(file_path);
        const data = parser.get();
        expect(data).toBeTruthy();
        expect(Array.isArray(data.collections)).toBe(true);

        parser.free();
    });

    test("roundtrip write", () => {
        const file_path = path.join(ROOT, files.osdb);
        roundtrip(
            () => new OsdbParser(),
            file_path,
            (first, second) => {
                expect(second.count).toBe(first.count);
                expect(second.collections[0].name).toBe(first.collections[0].name);
                expect(second.collections[0].beatmaps.length).toBe(first.collections[0].beatmaps.length);
            }
        );
    });
});
