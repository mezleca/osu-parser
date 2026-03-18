import { native } from "./lib/bindings";
import {
    BeatmapKey,
    BeatmapUpdate,
    OsdbData,
    OsdbKey,
    OsdbUpdate,
    OsuCollectionDb,
    OsuCollectionDbKey,
    OsuCollectionDbUpdate,
    OsuDbKey,
    OsuDbUpdate,
    OsuFileFormat,
    OsuLegacyDatabase,
    OsuReplay,
    OsuScoresDb
} from "./types/types";

class BaseParser {
    protected handle: bigint;

    constructor(handle: bigint) {
        this.handle = handle;
    }

    get ptr(): bigint {
        return this.handle;
    }

    protected clear_handle() {
        this.handle = 0n;
    }

    protected assert_handle() {
        if (this.handle === 0n) {
            throw new Error("parser handle is not valid");
        }
    }
}

export class BeatmapParser extends BaseParser {
    constructor() {
        super(native.create_beatmap_parser());
    }

    parse(location: string): this {
        this.assert_handle();
        const ok = native.beatmap_parser_parse(this.handle, location);

        if (!ok) {
            const message = native.beatmap_parser_last_error(this.handle);
            throw new Error(message ? `BeatmapParser.parse failed: ${message}` : "BeatmapParser.parse failed");
        }

        return this;
    }

    write(): void {
        this.assert_handle();
        const ok = native.beatmap_parser_write(this.handle);
        if (!ok) {
            throw new Error("BeatmapParser.write failed");
        }
    }

    get(): OsuFileFormat {
        this.assert_handle();
        return native.beatmap_parser_get(this.handle);
    }

    getByName<K extends BeatmapKey>(key: K): OsuFileFormat[K] {
        this.assert_handle();
        return native.beatmap_parser_get_by_name(this.handle, key) as OsuFileFormat[K];
    }

    update(patch: BeatmapUpdate): this {
        this.assert_handle();
        native.beatmap_parser_update(this.handle, patch);
        return this;
    }

    lastError(): string | null {
        this.assert_handle();
        return native.beatmap_parser_last_error(this.handle);
    }

    free() {
        native.free_beatmap_parser(this.handle);
        this.clear_handle();
    }
}

export class OsuDbParser extends BaseParser {
    constructor() {
        super(native.create_osu_db_parser());
    }

    parse(location: string): this {
        this.assert_handle();
        const ok = native.osu_db_parser_parse(this.handle, location);

        if (!ok) {
            const message = native.osu_db_parser_last_error(this.handle);
            throw new Error(message ? `OsuDbParser.parse failed: ${message}` : "OsuDbParser.parse failed");
        }

        return this;
    }

    write(): void {
        this.assert_handle();
        const ok = native.osu_db_parser_write(this.handle);
        if (!ok) {
            throw new Error("OsuDbParser.write failed");
        }
    }

    get(): OsuLegacyDatabase {
        this.assert_handle();
        return native.osu_db_parser_get(this.handle);
    }

    getByName<K extends OsuDbKey>(key: K): OsuLegacyDatabase[K] {
        this.assert_handle();
        return native.osu_db_parser_get_by_name(this.handle, key) as OsuLegacyDatabase[K];
    }

    update(patch: OsuDbUpdate): this {
        this.assert_handle();
        native.osu_db_parser_update(this.handle, patch);
        return this;
    }

    lastError(): string | null {
        this.assert_handle();
        return native.osu_db_parser_last_error(this.handle);
    }

    free() {
        native.free_osu_db_parser(this.handle);
        this.clear_handle();
    }
}

export class OsuCollectionDbParser extends BaseParser {
    constructor() {
        super(native.create_osu_collection_db_parser());
    }

    parse(location: string): this {
        this.assert_handle();
        const ok = native.osu_collection_db_parser_parse(this.handle, location);

        if (!ok) {
            const message = native.osu_collection_db_parser_last_error(this.handle);
            throw new Error(
                message ? `OsuCollectionDbParser.parse failed: ${message}` : "OsuCollectionDbParser.parse failed"
            );
        }

        return this;
    }

    write(): void {
        this.assert_handle();
        const ok = native.osu_collection_db_parser_write(this.handle);
        if (!ok) {
            throw new Error("OsuCollectionDbParser.write failed");
        }
    }

    get(): OsuCollectionDb {
        this.assert_handle();
        return native.osu_collection_db_parser_get(this.handle);
    }

    getByName<K extends OsuCollectionDbKey>(key: K): OsuCollectionDb[K] {
        this.assert_handle();
        return native.osu_collection_db_parser_get_by_name(this.handle, key) as OsuCollectionDb[K];
    }

    update(patch: OsuCollectionDbUpdate): this {
        this.assert_handle();
        native.osu_collection_db_parser_update(this.handle, patch);
        return this;
    }

    lastError(): string | null {
        this.assert_handle();
        return native.osu_collection_db_parser_last_error(this.handle);
    }

    free() {
        native.free_osu_collection_db_parser(this.handle);
        this.clear_handle();
    }
}

export class OsuScoresDbParser extends BaseParser {
    constructor() {
        super(native.create_osu_scores_db_parser());
    }

    parse(location: string): this {
        this.assert_handle();
        const ok = native.osu_scores_db_parser_parse(this.handle, location);

        if (!ok) {
            const message = native.osu_scores_db_parser_last_error(this.handle);
            throw new Error(message ? `OsuScoresDbParser.parse failed: ${message}` : "OsuScoresDbParser.parse failed");
        }

        return this;
    }

    write(): void {
        this.assert_handle();
        const ok = native.osu_scores_db_parser_write(this.handle);
        if (!ok) {
            throw new Error("OsuScoresDbParser.write failed");
        }
    }

    get(): OsuScoresDb {
        this.assert_handle();
        return native.osu_scores_db_parser_get(this.handle);
    }

    lastError(): string | null {
        this.assert_handle();
        return native.osu_scores_db_parser_last_error(this.handle);
    }

    free() {
        native.free_osu_scores_db_parser(this.handle);
        this.clear_handle();
    }
}

export class OsuReplayParser extends BaseParser {
    constructor() {
        super(native.create_osu_replay_parser());
    }

    parse(location: string): this {
        this.assert_handle();
        const ok = native.osu_replay_parser_parse(this.handle, location);

        if (!ok) {
            const message = native.osu_replay_parser_last_error(this.handle);
            throw new Error(message ? `OsuReplayParser.parse failed: ${message}` : "OsuReplayParser.parse failed");
        }

        return this;
    }

    write(): void {
        this.assert_handle();
        const ok = native.osu_replay_parser_write(this.handle);
        if (!ok) {
            throw new Error("OsuReplayParser.write failed");
        }
    }

    get(): OsuReplay {
        this.assert_handle();
        return native.osu_replay_parser_get(this.handle);
    }

    lastError(): string | null {
        this.assert_handle();
        return native.osu_replay_parser_last_error(this.handle);
    }

    free() {
        native.free_osu_replay_parser(this.handle);
        this.clear_handle();
    }
}

export class OsdbParser extends BaseParser {
    constructor() {
        super(native.create_osdb_parser());
    }

    parse(location: string): this {
        this.assert_handle();
        const ok = native.osdb_parser_parse(this.handle, location);

        if (!ok) {
            const message = native.osdb_parser_last_error(this.handle);
            throw new Error(message ? `OsdbParser.parse failed: ${message}` : "OsdbParser.parse failed");
        }

        return this;
    }

    write(): void {
        this.assert_handle();
        const ok = native.osdb_parser_write(this.handle);
        if (!ok) {
            throw new Error("OsdbParser.write failed");
        }
    }

    get(): OsdbData {
        this.assert_handle();
        return native.osdb_parser_get(this.handle);
    }

    getByName<K extends OsdbKey>(key: K): OsdbData[K] {
        this.assert_handle();
        return native.osdb_parser_get_by_name(this.handle, key) as OsdbData[K];
    }

    update(patch: OsdbUpdate): this {
        this.assert_handle();
        native.osdb_parser_update(this.handle, patch);
        return this;
    }

    lastError(): string | null {
        this.assert_handle();
        return native.osdb_parser_last_error(this.handle);
    }

    free() {
        native.free_osdb_parser(this.handle);
        this.clear_handle();
    }
}

export { native };
export * from "./types/types";
