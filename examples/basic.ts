import {
    BeatmapParser,
    OsuDbParser,
    OsuCollectionDbParser,
    OsuScoresDbParser,
    OsuReplayParser,
    OsdbParser
} from "../dist/index";

const beatmap = new BeatmapParser();
const osu_db = new OsuDbParser();
const collections = new OsuCollectionDbParser();
const scores = new OsuScoresDbParser();
const replay = new OsuReplayParser();
const osdb = new OsdbParser();

const print_handles = () => {
    console.log("handles", {
        beatmap: beatmap.ptr,
        osu_db: osu_db.ptr,
        collections: collections.ptr,
        scores: scores.ptr,
        replay: replay.ptr,
        osdb: osdb.ptr
    });
}

console.log("created handles:");
print_handles();

beatmap.free();
osu_db.free();
collections.free();
scores.free();
replay.free();
osdb.free();

console.log("freed handles:");

print_handles();
