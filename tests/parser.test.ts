import { afterAll, beforeAll, describe, expect, test } from "bun:test";
import { chromium } from "playwright";

import native_api from "../dist/index.js";
import fs from "fs";
import http from "http";
import path from "path";

const ROOT_DIR = path.join(import.meta.dir, "..");
const DATA_DIR = path.join(import.meta.dir, "data");
const WASM_WRAPPER_BUNDLE_PATH = path.join(ROOT_DIR, "dist", "browser", "wasm-wrapper.js");
const WASM_FACTORY_BUNDLE_PATH = path.join(ROOT_DIR, "dist", "browser", "osu-beatmap-parser.js");
const DATA_FILES = ["1.osu", "2.osu", "jeff.osu"];

const MIME_TYPES: Record<string, string> = {
    ".html": "text/html; charset=utf-8",
    ".js": "application/javascript; charset=utf-8",
    ".json": "application/json; charset=utf-8",
    ".osu": "text/plain; charset=utf-8",
    ".wasm": "application/wasm"
};

let static_server: http.Server | null = null;
let base_url = "";

const get_content_type = (file_path: string): string => {
    const ext = path.extname(file_path).toLowerCase();
    return MIME_TYPES[ext] || "application/octet-stream";
};

const create_static_server = async (): Promise<{ server: http.Server; base_url: string }> => {
    const server = http.createServer((req, res) => {
        const raw_url = req.url || "/";
        const clean_path = raw_url.split("?")[0] || "/";
        const decoded = decodeURIComponent(clean_path);
        const normalized = path.normalize(decoded).replace(/^(\.\.[/\\])+/, "");
        const target_path = normalized == "/" ? "/index.html" : normalized;
        const absolute_path = path.resolve(ROOT_DIR, `.${target_path}`);

        if (!absolute_path.startsWith(ROOT_DIR)) {
            res.writeHead(403).end("forbidden");
            return;
        }

        if (!fs.existsSync(absolute_path) || fs.statSync(absolute_path).isDirectory()) {
            res.writeHead(404).end("not found");
            return;
        }

        const content = fs.readFileSync(absolute_path);

        res.setHeader("content-type", get_content_type(absolute_path));
        res.writeHead(200).end(content);
    });

    await new Promise<void>((resolve) => {
        server.listen(0, "127.0.0.1", () => resolve());
    });

    const address = server.address();

    if (!address || typeof address == "string") {
        throw new Error("could not resolve static server address");
    }

    return {
        server,
        base_url: `http://127.0.0.1:${address.port}`
    };
};

const load_data = (name: string): Uint8Array => {
    const file_path = path.join(DATA_DIR, name);
    const buf = fs.readFileSync(file_path);
    return new Uint8Array(buf);
};

const run_common_tests = (label: string, api: any) => {
    describe(label, () => {
        test("get_property returns metadata", async () => {
            const data = load_data("1.osu");

            const title = await api.get_property(data, "Title");
            const artist = await api.get_property(data, "Artist");

            expect(title).toBe("Pinball");
            expect(artist).toBe("Nightcore");
        });

        test("parse returns expected sections", async () => {
            const data = load_data("1.osu");
            const parsed = await api.parse(data);

            expect(parsed.version).toBe(6);
            expect(parsed.Metadata.Creator).toBe("jericho2442");
            expect(parsed.General.Mode).toBe(0);
            expect(parsed.Difficulty.SliderTickRate).toBe(2);
            expect(parsed.Events.background?.filename).toBe("nightcore III.jpg");
        });

        test("slider defaults are normalized", async () => {
            const data = load_data("1.osu");
            const parsed = await api.parse(data);
            const slider = parsed.HitObjects.find((obj: any) => (obj.type & 2) != 0);

            if (!slider) return;

            expect(slider.edgeSounds.length).toBe(slider.slides + 1);
            expect(slider.edgeSets.length).toBe(slider.slides + 1);
            expect(slider.hitSample).toBeTruthy();
        });

        test("timing points preserve red/green ordering at same time", async () => {
            const data = load_data("jeff.osu");
            const parsed = await api.parse(data);
            const points = parsed.TimingPoints;
            const at_225 = points.filter((tp: any) => tp.time === 225);

            expect(at_225.length).toBeGreaterThanOrEqual(2);
            expect(at_225[0].beatLength).toBeCloseTo(666.666666666667, 6);
            expect(at_225[0].uninherited).toBe(1);
            expect(at_225[1].beatLength).toBe(-200);
            expect(at_225[1].uninherited).toBe(0);
        });

        for (const name of DATA_FILES) {
            test(`parse works for ${name}`, async () => {
                const data = load_data(name);
                const parsed = await api.parse(data);
                expect(parsed).toBeTruthy();
            });
        }
    });
};

const run_browser_wasm_tests = () => {
    describe("wasm-browser", () => {
        beforeAll(async () => {
            const created = await create_static_server();
            static_server = created.server;
            base_url = created.base_url;
        });

        afterAll(async () => {
            if (!static_server) return;

            await new Promise<void>((resolve, reject) => {
                static_server?.close((err) => {
                    if (err) {
                        reject(err);
                        return;
                    }

                    resolve();
                });
            });
        });

        test("parse in worker", async () => {
            if (!fs.existsSync(WASM_WRAPPER_BUNDLE_PATH)) {
                throw new Error("missing wasm wrapper bundle. run: bun run compile:wasm");
            }

            if (!fs.existsSync(WASM_FACTORY_BUNDLE_PATH)) {
                throw new Error("missing wasm factory bundle. run: bun run compile:wasm");
            }

            const browser = await chromium.launch({ headless: true });
            const page = await browser.newPage();

            try {
                await page.goto(`${base_url}/tests/browser-harness.html`, { waitUntil: "domcontentloaded" });

                const summary = await page.evaluate(async () => {
                    const parser = (globalThis as any).beatmap_parser;
                    if (!parser) {
                        throw new Error("beatmap_parser global not found");
                    }

                    await parser.init_wasm();

                    const collected = [];
                    for (const name of ["1.osu", "2.osu", "jeff.osu"]) {
                        const response = await fetch(`/tests/data/${name}`);
                        const buffer = await response.arrayBuffer();
                        const data = new Uint8Array(buffer);
                        const parsed = await parser.parse(data);
                        collected.push({
                            name,
                            version: parsed.version,
                            creator: parsed.Metadata.Creator
                        });
                    }

                    return {
                        ready: parser.is_wasm_ready(),
                        collected
                    };
                });

                expect(summary.ready).toBe(true);
                expect(summary.collected.length).toBe(3);
                expect(summary.collected[0].version).toBe(6);
                expect(summary.collected[0].creator).toBe("jericho2442");
            } finally {
                await page.close();
                await browser.close();
            }
        }, 60_000);
    });
};

run_common_tests("native", native_api);

if (process.env.RUN_BROWSER_WASM_TESTS == "1") {
    run_browser_wasm_tests();
} else {
    test.skip("wasm-browser", () => {});
}
