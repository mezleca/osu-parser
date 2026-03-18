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

describe("beatmap parser", () => {
    test("parse beatmaps/243.osu (v3)", () => {
        const parser = new BeatmapParser();
        const file_path = path.join(ROOT, "beatmaps/243.osu");

        expect(() => parser.parse(file_path)).not.toThrow();

        const data = parser.get();

        if (data.version !== 3) {
            // debug for windows CI
            console.log("version", data.version, "type", typeof data.version);
            console.log("raw", (data as any).__version_raw);
            console.log("descriptor", Object.getOwnPropertyDescriptor(data, "version"));
            console.log("keys", Object.keys(data).slice(0, 10));
        }

        expect(data.version).toBe(3);
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

        parser.free();
    });

    test("parse beatmaps/1636774.osu (v14)", () => {
        const parser = new BeatmapParser();
        const file_path = path.join(ROOT, "beatmaps/1636774.osu");

        expect(() => parser.parse(file_path)).not.toThrow();

        const data = parser.get();

        expect(data.version).toBe(14);
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

        parser.free();
    });

    test("parse beatmaps/2303521.osu (v14 + video)", () => {
        const parser = new BeatmapParser();
        const file_path = path.join(ROOT, "beatmaps/2303521.osu");

        expect(() => parser.parse(file_path)).not.toThrow();

        const data = parser.get();

        expect(data.version).toBe(14);
        expect(data.General.StackLeniency).toBeCloseTo(0.3, 6);
        expect(data.Metadata.Title).toBe("Diamonds From Sierra Leone");
        expect(data.Metadata.Creator).toBe("Herazu");
        expect(data.Events.background?.filename).toBe("y.png");
        expect(data.Events.video?.filename).toBe("Diamond Loop.mp4");
        expect(data.TimingPoints.length).toBeGreaterThan(1);
        expect(data.TimingPoints[0].time).toBe(12832);
        expect(data.TimingPoints[0].beatLength).toBeCloseTo(617.919670442843, 6);

        parser.free();
    });
});

describe("osu!.db parser", () => {
    test("parse file", () => {
        const parser = new OsuDbParser();
        const file_path = path.join(ROOT, files.osu_db);

        expect(() => parser.parse(file_path)).not.toThrow();
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

        parser.free();
    });

    test("roundtrip write", () => {
        const file_path = path.join(ROOT, files.osu_db);
        roundtrip(
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
});

describe("collection.db parser", () => {
    test("parse file", () => {
        const parser = new OsuCollectionDbParser();
        const file_path = path.join(ROOT, files.collection_db);

        expect(() => parser.parse(file_path)).not.toThrow();
        const data = parser.get();
        expect(data).toBeTruthy();
        expect(data.collections_count).toBe(2);
        expect(Array.isArray(data.collections)).toBe(true);

        const first = data.collections[0];
        expect(first.name).toBe("glass beach");
        expect(first.beatmaps_count).toBe(10);
        expect(first.beatmap_md5[0]).toBe("6737a1d011bd8ea8b008aff294147f33");

        parser.free();
    });

    test("roundtrip write", () => {
        const file_path = path.join(ROOT, files.collection_db);
        roundtrip(
            () => new OsuCollectionDbParser(),
            file_path,
            (first, second) => {
                expect(second.collections_count).toBe(first.collections_count);
                expect(second.collections[0].name).toBe(first.collections[0].name);
                expect(second.collections[0].beatmap_md5[0]).toBe(first.collections[0].beatmap_md5[0]);
            }
        );
    });
});

describe("scores.db parser", () => {
    test("parse file", () => {
        const parser = new OsuScoresDbParser();
        const file_path = path.join(ROOT, files.scores_db);

        expect(() => parser.parse(file_path)).not.toThrow();
        const data = parser.get();
        expect(data).toBeTruthy();
        expect(Array.isArray(data.beatmaps)).toBe(true);

        parser.free();
    });
});

describe("replay parser", () => {
    for (const name of files.replays) {
        test(`parse ${name}`, () => {
            const parser = new OsuReplayParser();
            const file_path = path.join(ROOT, name);

            expect(() => parser.parse(file_path)).not.toThrow();
            const data = parser.get();
            expect(data).toBeTruthy();
            expect(typeof data.replay_md5).toBe("string");

            parser.free();
        });
    }
});

describe("osdb parser", () => {
    test("parse file", () => {
        const parser = new OsdbParser();
        const file_path = path.join(ROOT, files.osdb);

        expect(() => parser.parse(file_path)).not.toThrow();
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

describe("concurrency", () => {
    test("parse in parallel with separate instances", async () => {
        const tasks: Array<() => void> = [];

        for (const name of files.beatmaps) {
            tasks.push(() => {
                const parser = new BeatmapParser();
                parser.parse(path.join(ROOT, name));
                expect(parser.get()).toBeTruthy();
                parser.free();
            });
        }

        tasks.push(() => {
            const parser = new OsuDbParser();
            parser.parse(path.join(ROOT, files.osu_db));
            expect(parser.get()).toBeTruthy();
            parser.free();
        });

        tasks.push(() => {
            const parser = new OsuCollectionDbParser();
            parser.parse(path.join(ROOT, files.collection_db));
            expect(parser.get()).toBeTruthy();
            parser.free();
        });

        tasks.push(() => {
            const parser = new OsuScoresDbParser();
            parser.parse(path.join(ROOT, files.scores_db));
            expect(parser.get()).toBeTruthy();
            parser.free();
        });

        tasks.push(() => {
            const parser = new OsdbParser();
            parser.parse(path.join(ROOT, files.osdb));
            expect(parser.get()).toBeTruthy();
            parser.free();
        });

        await Promise.all(tasks.map((task) => Promise.resolve().then(task)));
    });
});

describe("errors", () => {
    test("lastError returns message on parse failure", () => {
        const bad_paths = {
            beatmap: path.join(ROOT, "beatmaps/does-not-exist.osu"),
            osu_db: path.join(ROOT, "osu/osu-missing.db"),
            collection_db: path.join(ROOT, "collections/collection-missing.db"),
            scores_db: path.join(ROOT, "scores/scores-missing.db"),
            replay: path.join(ROOT, "replays/missing.osr"),
            osdb: path.join(ROOT, "collections/missing.osdb")
        };

        const check = <
            T extends { parse: (path: string) => unknown; lastError: () => string | null; free: () => void }
        >(
            parser: T,
            file_path: string
        ) => {
            expect(() => parser.parse(file_path)).toThrow();
            expect(typeof parser.lastError()).toBe("string");
            parser.free();
        };

        check(new BeatmapParser(), bad_paths.beatmap);
        check(new OsuDbParser(), bad_paths.osu_db);
        check(new OsuCollectionDbParser(), bad_paths.collection_db);
        check(new OsuScoresDbParser(), bad_paths.scores_db);
        check(new OsuReplayParser(), bad_paths.replay);
        check(new OsdbParser(), bad_paths.osdb);
    });
});
