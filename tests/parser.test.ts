import { describe, expect, test } from "bun:test";
import fs from "fs";
import os from "os";
import path from "path";

import {
    BeatmapParser,
    OsuCollectionDbParser,
    OsuDbParser,
    OsuReplayParser,
    OsuScoresDbParser,
    OsdbParser
} from "../dist/index";
import { Worker } from "worker_threads";

const ROOT = path.join(import.meta.dir, "data");

const files = {
    beatmaps: ["beatmaps/1636774.osu", "beatmaps/2303521.osu", "beatmaps/243.osu"],
    osu_db: "osu/osu!.db",
    collection_db: "collections/collection.db",
    osdb: "collections/mathi.osdb",
    scores_db: "scores/scores.db",
    replays: ["replays/1.osr", "replays/2.osr"]
};

const withTempCopy = (source: string) => {
    const temp_dir = fs.mkdtempSync(path.join(os.tmpdir(), "osu-parser-"));
    const temp_path = path.join(temp_dir, path.basename(source));
    fs.copyFileSync(source, temp_path);
    return { temp_dir, temp_path };
};

const roundtrip = async <T>(
    create: () => {
        parse: (location: string) => Promise<unknown>;
        get: () => T;
        write: () => Promise<void>;
        free: () => void;
    },
    source: string,
    assert: (first: T, second: T) => void
) => {
    const { temp_dir, temp_path } = withTempCopy(source);
    let parser: ReturnType<typeof create> | null = null;
    let parser2: ReturnType<typeof create> | null = null;

    try {
        parser = create();
        await parser.parse(temp_path);
        const first = parser.get();
        await parser.write();
        parser.free();
        parser = null;

        parser2 = create();
        await parser2.parse(temp_path);
        const second = parser2.get();
        parser2.free();
        parser2 = null;

        assert(first, second);
    } finally {
        if (parser) {
            parser.free();
        }
        if (parser2) {
            parser2.free();
        }
        fs.rmSync(temp_dir, { recursive: true, force: true });
    }
};

describe("beatmap parser", () => {
    test("parse beatmaps/243.osu (v3)", async () => {
        const parser = new BeatmapParser();
        try {
            const file_path = path.join(ROOT, "beatmaps/243.osu");
            await parser.parse(file_path);

            const data = parser.get();

            expect(data.version).toBe("v3");
            expect(data.General.AudioFilename).toBe("Marisa wa Taihen na Mono wo Nusunde Ikimashita.mp3");
            expect(data.General.PreviewTime).toBe(91764);
            expect(data.General.Countdown).toBe(1);
            expect(data.Metadata.Title).toBe("Marisa wa Taihen na Mono wo Nusunde Ikimashita");
            expect(data.Metadata.Artist).toBe("IOSYS");
            expect(data.Metadata.Creator).toBe("DJPop");
            expect(data.Difficulty.SliderMultiplier).toBeCloseTo(0.5, 5);
            expect(data.Events.background?.filename).toBe("Marisa wa Taihen na Mono wo Nusunde Ikimashita.png");
            expect(data.TimingPoints.length).toBe(1);
            expect(data.TimingPoints[0].time).toBe(0);
            expect(data.TimingPoints[0].beatLength).toBeCloseTo(352.941176470588, 6);
            expect(data.HitObjects.length).toBeGreaterThan(0);

            const slider = data.HitObjects.find((obj) => (obj.type & 2) !== 0);
            expect(slider).toBeTruthy();

            if (slider) {
                expect(slider.edgeSounds.length).toBe(slider.slides + 1);
                expect(slider.edgeSets.length).toBe(slider.slides + 1);
                expect(slider.hitSample).toBeTruthy();
            }
        } finally {
            parser.free();
        }
    });

    test("parse beatmaps/1636774.osu (v14)", async () => {
        const parser = new BeatmapParser();
        try {
            const file_path = path.join(ROOT, "beatmaps/1636774.osu");
            await parser.parse(file_path);

            const data = parser.get();

            expect(data.version).toBe("v14");
            expect(data.General.SampleSet).toBe("Soft");
            expect(data.General.Mode).toBe(0);
            expect(data.Editor.BeatDivisor).toBe(4);
            expect(data.Editor.GridSize).toBe(16);
            expect(data.Metadata.Creator).toBe("Leaf");
            expect(data.Events.background?.filename).toBe("bg22.jpg");
            expect(data.Events.breaks.length).toBe(1);
            expect(data.Events.breaks[0].startTime).toBe(197822);
            expect(data.Events.breaks[0].endTime).toBe(207343);
            expect(data.TimingPoints.length).toBeGreaterThan(1);
            expect(data.TimingPoints[0].time).toBe(770);
            expect(data.TimingPoints[0].beatLength).toBeCloseTo(315.789473684211, 6);
        } finally {
            parser.free();
        }
    });

    test("parse beatmaps/2303521.osu (v14 + video)", async () => {
        const parser = new BeatmapParser();
        try {
            const file_path = path.join(ROOT, "beatmaps/2303521.osu");
            await parser.parse(file_path);

            const data = parser.get();

            expect(data.version).toBe("v14");
            expect(data.General.StackLeniency).toBeCloseTo(0.3, 6);
            expect(data.Metadata.Title).toBe("Diamonds From Sierra Leone");
            expect(data.Metadata.Creator).toBe("Herazu");
            expect(data.Events.background?.filename).toBe("y.png");
            expect(data.Events.video?.filename).toBe("Diamond Loop.mp4");
            expect(data.TimingPoints.length).toBeGreaterThan(1);
            expect(data.TimingPoints[0].time).toBe(12832);
            expect(data.TimingPoints[0].beatLength).toBeCloseTo(617.919670442843, 6);
        } finally {
            parser.free();
        }
    });

    test("roundtrip write", async () => {
        const file_path = path.join(ROOT, "beatmaps/1636774.osu");
        await roundtrip(
            () => new BeatmapParser(),
            file_path,
            (first, second) => {
                expect(second.version).toBe(first.version);
                expect(second.General.AudioFilename).toBe(first.General.AudioFilename);
                expect(second.Metadata.Creator).toBe(first.Metadata.Creator);
                expect(second.Difficulty.SliderMultiplier).toBeCloseTo(first.Difficulty.SliderMultiplier, 6);
                expect(second.Events.background?.filename).toBe(first.Events.background?.filename);
                expect(second.HitObjects.length).toBe(first.HitObjects.length);
            }
        );
    });

    test("get_by_name", async () => {
        const parser = new BeatmapParser();
        try {
            const file_path = path.join(ROOT, "beatmaps/1636774.osu");
            await parser.parse(file_path);
            expect(parser.get_by_name("version")).toBe("v14");
            expect(parser.get_by_name("General")).toBeTruthy();
            expect(parser.get_by_name("Events")).toBeTruthy();
            expect(parser.get_by_name("TimingPoints")).toBeTruthy();
            expect(parser.get_by_name("HitObjects")).toBeTruthy();
        } finally {
            parser.free();
        }
    });

    test("update fields and arrays", async () => {
        const parser = new BeatmapParser();
        try {
            const file_path = path.join(ROOT, "beatmaps/1636774.osu");
            await parser.parse(file_path);
            const before = parser.get();
            parser.update({
                General: { AudioFilename: "changed.mp3" },
                Events: { background: null },
                Colours: { SliderBorder: [255, 0, 0] }
            });
            parser.update({
                TimingPoints: [
                    {
                        time: 123,
                        beatLength: 500,
                        meter: 4,
                        sampleSet: 0,
                        sampleIndex: 0,
                        volume: 100,
                        uninherited: 1,
                        effects: 0
                    }
                ],
                Colours: { Combos: [[1, 2, 3]] },
                HitObjects: [before.HitObjects[0]]
            });

            const after = parser.get();

            expect(after.General.AudioFilename).toBe("changed.mp3");
            expect(after.Events.background).toBeNull();
            expect(after.TimingPoints.length).toBe(1);
            expect(after.Colours.SliderBorder).toEqual([255, 0, 0]);
            expect(after.Colours.Combos.length).toBe(1);
            expect(after.HitObjects.length).toBe(1);
        } finally {
            parser.free();
        }
    });

    test("update errors", async () => {
        const parser = new BeatmapParser();
        try {
            const file_path = path.join(ROOT, "beatmaps/1636774.osu");
            await parser.parse(file_path);
            expect(() => parser.update({ TimingPoints: {} as unknown as any })).toThrow();
            expect(() => parser.update({ HitObjects: {} as unknown as any })).toThrow();
        } finally {
            parser.free();
        }
    });
});

describe("osu!.db parser", () => {
    test("parse file", async () => {
        const parser = new OsuDbParser();
        try {
            const file_path = path.join(ROOT, files.osu_db);
            await parser.parse(file_path);
            const data = parser.get();
            expect(data).toBeTruthy();
            expect(data.version).toBe(20251102);
            expect(data.player_name).toBe("mzle");
            expect(data.beatmaps_count).toBe(47);
            expect(Array.isArray(data.beatmaps)).toBe(true);

            const first = data.beatmaps[0];
            expect(first.artist).toBe("FRAM");
            expect(first.title).toBe("Step for Joy");
            expect(first.md5).toBe("8e66c5e88adb59774e4eccca702fe242");
            expect(first.star_rating_standard.length).toBeGreaterThan(0);
            expect(first.star_rating_standard[0].mod_combination).toBe(0);
            expect(first.star_rating_standard[0].star_rating).toBeCloseTo(4.707488, 5);
        } finally {
            parser.free();
        }
    });

    test("roundtrip write", async () => {
        const file_path = path.join(ROOT, files.osu_db);
        await roundtrip(
            () => new OsuDbParser(),
            file_path,
            (first, second) => {
                expect(second.version).toBe(first.version);
                expect(second.player_name).toBe(first.player_name);
                expect(second.beatmaps_count).toBe(first.beatmaps_count);
                expect(second.beatmaps[0].md5).toBe(first.beatmaps[0].md5);
            }
        );
    });

    test("get_by_name", async () => {
        const parser = new OsuDbParser();
        try {
            const file_path = path.join(ROOT, files.osu_db);
            await parser.parse(file_path);
            expect(parser.get_by_name("version")).toBe(20251102);
            expect(parser.get_by_name("player_name")).toBe("mzle");
            expect(parser.get_by_name("beatmaps")).toBeTruthy();
        } finally {
            parser.free();
        }
    });

    test("update fields and arrays", async () => {
        const parser = new OsuDbParser();
        try {
            const file_path = path.join(ROOT, files.osu_db);
            await parser.parse(file_path);
            const before = parser.get();

            const beatmaps = [...before.beatmaps];
            beatmaps[0] = { ...beatmaps[0], artist: "Updated Artist" };
            parser.update({ player_name: "updated", beatmaps });

            const after = parser.get();
            expect(after.player_name).toBe("updated");
            expect(after.beatmaps[0].artist).toBe("Updated Artist");
            expect(after.beatmaps_count).toBe(before.beatmaps_count);
        } finally {
            parser.free();
        }
    });

    test("update errors", async () => {
        const parser = new OsuDbParser();
        try {
            const file_path = path.join(ROOT, files.osu_db);
            await parser.parse(file_path);
            expect(() => parser.update({ beatmaps: {} as unknown as any })).toThrow();
        } finally {
            parser.free();
        }
    });

    test("filter helpers", async () => {
        const parser = new OsuDbParser();
        try {
            const file_path = path.join(ROOT, files.osu_db);
            await parser.parse(file_path);
            const data = parser.get();
            const first = data.beatmaps[0];
            const safe = (value: string) => value.replaceAll('"', "");

            const mode_alias = (mode: number) => {
                switch (mode) {
                    case 1:
                        return "t";
                    case 2:
                        return "c";
                    case 3:
                        return "m";
                    case 0:
                    default:
                        return "o";
                }
            };

            const status_alias = (status: number) => {
                switch (status) {
                    case 4:
                        return "r";
                    case 5:
                        return "a";
                    case 2:
                        return "p";
                    case 1:
                        return "n";
                    case 0:
                        return "u";
                    case 7:
                        return "l";
                    default:
                        return "u";
                }
            };

            const by_md5 = parser.filter_by_properties({ md5: first.md5 });
            expect(by_md5.length).toBe(1);
            expect(by_md5[0].md5).toBe(first.md5);

            const by_query = parser.filter_by_properties({ query: `${first.artist} ${first.title}` });
            expect(by_query.length).toBeGreaterThan(0);

            const advanced_query = [
                `artist="${safe(first.artist)}"`,
                `title="${safe(first.title)}"`,
                `mapper="${safe(first.creator)}"`,
                `diff="${safe(first.difficulty)}"`,
                `ar>=${Math.max(0, first.approach_rate - 0.01)}`,
                `cs>=${Math.max(0, first.circle_size - 0.01)}`,
                `od>=${Math.max(0, first.overall_difficulty - 0.01)}`,
                `hp>=${Math.max(0, first.hp_drain - 0.01)}`,
                `length>=${Math.max(0, first.total_time / 1000 - 1)}`,
                `mode=${mode_alias(first.mode)}`,
                `status=${status_alias(first.ranked_status)},l`
            ].join(" ");

            const by_advanced = parser.filter_by_properties({ query: advanced_query });
            expect(by_advanced.some((beatmap) => beatmap.md5 === first.md5)).toBe(true);

            const md5_list = parser.filter_md5_by_properties({ md5: first.md5 });
            expect(md5_list).toEqual([first.md5]);

            const diff_ids = parser.filter_ids_by_properties({ md5: first.md5 });
            expect(diff_ids).toEqual([first.difficulty_id]);

            const set_ids = parser.filter_ids_by_properties({ md5: first.md5, id_type: "beatmap_id" });
            expect(set_ids).toEqual([first.beatmap_id]);
        } finally {
            parser.free();
        }
    });
});

describe("collection.db parser", () => {
    test("parse file", async () => {
        const parser = new OsuCollectionDbParser();
        try {
            const file_path = path.join(ROOT, files.collection_db);
            await parser.parse(file_path);
            const data = parser.get();
            expect(data).toBeTruthy();
            expect(data.collections_count).toBe(2);
            expect(Array.isArray(data.collections)).toBe(true);

            const first = data.collections[0];
            expect(first.name).toBe("glass beach");
            expect(first.beatmaps_count).toBe(10);
            expect(first.beatmap_md5[0]).toBe("6737a1d011bd8ea8b008aff294147f33");
        } finally {
            parser.free();
        }
    });

    test("roundtrip write", async () => {
        const file_path = path.join(ROOT, files.collection_db);
        await roundtrip(
            () => new OsuCollectionDbParser(),
            file_path,
            (first, second) => {
                expect(second.collections_count).toBe(first.collections_count);
                expect(second.collections[0].name).toBe(first.collections[0].name);
                expect(second.collections[0].beatmap_md5[0]).toBe(first.collections[0].beatmap_md5[0]);
            }
        );
    });

    test("get_by_name", async () => {
        const parser = new OsuCollectionDbParser();
        try {
            const file_path = path.join(ROOT, files.collection_db);
            await parser.parse(file_path);
            expect(parser.get_by_name("collections_count")).toBe(2);
            expect(parser.get_by_name("collections")).toBeTruthy();
        } finally {
            parser.free();
        }
    });

    test("update fields and arrays", async () => {
        const parser = new OsuCollectionDbParser();
        try {
            const file_path = path.join(ROOT, files.collection_db);
            await parser.parse(file_path);
            const before = parser.get();

            const collections = [...before.collections];
            collections[0] = { ...collections[0], name: "updated-collection" };
            collections[0] = { ...collections[0], beatmap_md5: [...collections[0].beatmap_md5, "abc123"] };
            collections.push({ name: "new-collection", beatmaps_count: 0, beatmap_md5: [] });
            collections.splice(1, 1);
            parser.update({ collections });

            const after = parser.get();
            expect(after.collections_count).toBe(before.collections_count);
            expect(after.collections[0].name).toBe("updated-collection");
            expect(after.collections[0].beatmap_md5).toContain("abc123");
            expect(after.collections.length).toBe(before.collections.length);
        } finally {
            parser.free();
        }
    });

    test("update errors", async () => {
        const parser = new OsuCollectionDbParser();
        try {
            const file_path = path.join(ROOT, files.collection_db);
            await parser.parse(file_path);
            expect(() => parser.update({ collections: {} as unknown as any })).toThrow();
        } finally {
            parser.free();
        }
    });
});

describe("osdb parser", () => {
    test("parse file", async () => {
        const parser = new OsdbParser();
        try {
            const file_path = path.join(ROOT, files.osdb);
            await parser.parse(file_path);
            const data = parser.get();
            expect(data).toBeTruthy();
            expect(Array.isArray(data.collections)).toBe(true);
        } finally {
            parser.free();
        }
    });

    test("roundtrip write", async () => {
        const file_path = path.join(ROOT, files.osdb);
        await roundtrip(
            () => new OsdbParser(),
            file_path,
            (first, second) => {
                expect(second.count).toBe(first.count);
                expect(second.collections[0].name).toBe(first.collections[0].name);
                expect(second.collections[0].beatmaps.length).toBe(first.collections[0].beatmaps.length);
            }
        );
    });

    test("get_by_name", async () => {
        const parser = new OsdbParser();
        try {
            const file_path = path.join(ROOT, files.osdb);
            await parser.parse(file_path);
            expect(parser.get_by_name("count")).toBeGreaterThan(0);
            expect(parser.get_by_name("collections")).toBeTruthy();
        } finally {
            parser.free();
        }
    });

    test("update fields and arrays", async () => {
        const parser = new OsdbParser();
        try {
            const file_path = path.join(ROOT, files.osdb);
            await parser.parse(file_path);
            const before = parser.get();

            const collections = [...before.collections];
            collections[0] = { ...collections[0], name: "updated-collection" };
            collections[0] = {
                ...collections[0],
                hash_only_beatmaps: [...collections[0].hash_only_beatmaps, "deadbeef"]
            };
            collections.push({
                name: "new-collection",
                online_id: 0,
                beatmaps: [],
                hash_only_beatmaps: []
            });
            parser.update({ last_editor: "updated", collections });

            const after = parser.get();
            expect(after.last_editor).toBe("updated");
            expect(after.collections[0].name).toBe("updated-collection");
            expect(after.collections[0].hash_only_beatmaps).toContain("deadbeef");
            expect(after.collections.length).toBe(before.collections.length + 1);
        } finally {
            parser.free();
        }
    });

    test("update errors", async () => {
        const parser = new OsdbParser();
        try {
            const file_path = path.join(ROOT, files.osdb);
            await parser.parse(file_path);
            expect(() => parser.update({ collections: {} as unknown as any })).toThrow();
        } finally {
            parser.free();
        }
    });
});

describe("replay parser", () => {
    for (const name of files.replays) {
        test(`parse ${name}`, async () => {
            const parser = new OsuReplayParser();
            try {
                const file_path = path.join(ROOT, name);
                await parser.parse(file_path);
                const data = parser.get();
                expect(data).toBeTruthy();
                expect(typeof data.replay_md5).toBe("string");
            } finally {
                parser.free();
            }
        });
    }

    test("roundtrip write", async () => {
        const file_path = path.join(ROOT, files.replays[0]);
        await roundtrip(
            () => new OsuReplayParser(),
            file_path,
            (first, second) => {
                expect(second.replay_md5).toBe(first.replay_md5);
                expect(second.player_name).toBe(first.player_name);
                expect(second.replay_data_length).toBe(first.replay_data_length);
                expect(second.replay_data.length).toBe(first.replay_data.length);
                expect(second.online_score_id).toBe(first.online_score_id);
            }
        );
    });
});

describe("concurrency", () => {
    test("parse in parallel with separate instances", async () => {
        const module_path = path.join(import.meta.dir, "..", "dist", "index.js");
        const worker_source = `
            const { parentPort, workerData } = require("worker_threads");
            const parsers = require(workerData.modulePath);

            (async () => {
                let parser;
                switch (workerData.kind) {
                    case "beatmap":
                        parser = new parsers.BeatmapParser();
                        break;
                    case "osu_db":
                        parser = new parsers.OsuDbParser();
                        break;
                    case "collection_db":
                        parser = new parsers.OsuCollectionDbParser();
                        break;
                    case "scores_db":
                        parser = new parsers.OsuScoresDbParser();
                        break;
                    case "osdb":
                        parser = new parsers.OsdbParser();
                        break;
                    default:
                        throw new Error("unknown parser kind");
                }

                await parser.parse(workerData.filePath);
                const ok = !!(parser.get());
                parser.free();
                parentPort.postMessage(ok);
            })().catch(() => {
                parentPort.postMessage(false);
            });
        `;

        const run_worker = (kind: string, file_path: string) =>
            new Promise<boolean>((resolve, reject) => {
                const worker = new Worker(worker_source, {
                    eval: true,
                    workerData: { kind, filePath: file_path, modulePath: module_path }
                });
                worker.on("message", (value) => resolve(Boolean(value)));
                worker.on("error", reject);
                worker.on("exit", (code) => {
                    if (code !== 0) {
                        reject(new Error(`worker exited with code ${code}`));
                    }
                });
            });

        const tasks: Promise<boolean>[] = [];

        for (const name of files.beatmaps) {
            tasks.push(run_worker("beatmap", path.join(ROOT, name)));
        }

        tasks.push(run_worker("osu_db", path.join(ROOT, files.osu_db)));
        tasks.push(run_worker("collection_db", path.join(ROOT, files.collection_db)));
        tasks.push(run_worker("scores_db", path.join(ROOT, files.scores_db)));
        tasks.push(run_worker("osdb", path.join(ROOT, files.osdb)));

        const results = await Promise.all(tasks);
        for (const ok of results) {
            expect(ok).toBe(true);
        }
    });
});

describe("errors", () => {
    test("last_error returns message on parse failure", async () => {
        const bad_paths = {
            beatmap: path.join(ROOT, "beatmaps/does-not-exist.osu"),
            osu_db: path.join(ROOT, "osu/osu-missing.db"),
            collection_db: path.join(ROOT, "collections/collection-missing.db"),
            scores_db: path.join(ROOT, "scores/scores-missing.db"),
            replay: path.join(ROOT, "replays/missing.osr"),
            osdb: path.join(ROOT, "collections/missing.osdb")
        };

        const check = async <
            T extends {
                parse: (path: string) => Promise<unknown>;
                last_error: () => string | null;
                free: () => void;
            }
        >(
            parser: T,
            file_path: string
        ) => {
            await expect(parser.parse(file_path)).rejects.toThrow();
            expect(typeof parser.last_error()).toBe("string");
            parser.free();
        };

        await check(new BeatmapParser(), bad_paths.beatmap);
        await check(new OsuDbParser(), bad_paths.osu_db);
        await check(new OsuCollectionDbParser(), bad_paths.collection_db);
        await check(new OsuScoresDbParser(), bad_paths.scores_db);
        await check(new OsuReplayParser(), bad_paths.replay);
        await check(new OsdbParser(), bad_paths.osdb);
    });
});

describe("scores.db parser", () => {
    test("parse file", async () => {
        const parser = new OsuScoresDbParser();
        try {
            const file_path = path.join(ROOT, files.scores_db);
            await parser.parse(file_path);
            const data = parser.get();
            expect(data).toBeTruthy();
            expect(Array.isArray(data.beatmaps)).toBe(true);
        } finally {
            parser.free();
        }
    });

    test("roundtrip write", async () => {
        const file_path = path.join(ROOT, files.scores_db);
        await roundtrip(
            () => new OsuScoresDbParser(),
            file_path,
            (first, second) => {
                expect(second.beatmaps_count).toBe(first.beatmaps_count);
                if (first.beatmaps.length > 0) {
                    expect(second.beatmaps[0].beatmap_md5).toBe(first.beatmaps[0].beatmap_md5);
                    expect(second.beatmaps[0].scores.length).toBe(first.beatmaps[0].scores.length);
                }
            }
        );
    });
});
