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

type ParserCoreFns<TGet> = {
    name: string;
    create: () => bigint;
    free: (handle: bigint) => void;
    parse: (handle: bigint, location: string) => Promise<boolean>;
    write: (handle: bigint) => Promise<boolean>;
    get: (handle: bigint) => TGet;
    last_error: (handle: bigint) => string | null;
};

type ParserUpdateFns<TGet, TKey extends string, TUpdate> = ParserCoreFns<TGet> & {
    getByName: (handle: bigint, key: TKey) => unknown;
    update: (handle: bigint, patch: TUpdate) => boolean;
};

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

class ParserBase<TGet> extends BaseParser {
    protected fns: ParserCoreFns<TGet>;

    constructor(fns: ParserCoreFns<TGet>) {
        super(fns.create());
        this.fns = fns;
    }

    async parse(location: string): Promise<this> {
        this.assert_handle();
        const ok = await this.fns.parse(this.handle, location);

        if (!ok) {
            const message = this.fns.last_error(this.handle);
            throw new Error(message ? `${this.fns.name}.parse failed: ${message}` : `${this.fns.name}.parse failed`);
        }

        return this;
    }

    async write(): Promise<void> {
        this.assert_handle();
        const ok = await this.fns.write(this.handle);
        if (!ok) {
            throw new Error(`${this.fns.name}.write failed`);
        }
    }

    get(): TGet {
        this.assert_handle();
        return this.fns.get(this.handle);
    }

    last_error(): string | null {
        this.assert_handle();
        return this.fns.last_error(this.handle);
    }

    free(): void {
        this.fns.free(this.handle);
        this.clear_handle();
    }
}

class ParserWithUpdateAndGetByName<
    TGet extends object,
    TKey extends keyof TGet & string,
    TUpdate
> extends ParserBase<TGet> {
    protected fns: ParserUpdateFns<TGet, TKey, TUpdate>;

    constructor(fns: ParserUpdateFns<TGet, TKey, TUpdate>) {
        super(fns);
        this.fns = fns;
    }

    get_by_name<K extends TKey>(key: K): TGet[K] {
        this.assert_handle();
        return this.fns.getByName(this.handle, key) as TGet[K];
    }

    update(patch: TUpdate): this {
        this.assert_handle();
        this.fns.update(this.handle, patch);
        return this;
    }
}

export class BeatmapParser extends ParserWithUpdateAndGetByName<OsuFileFormat, BeatmapKey, BeatmapUpdate> {
    constructor() {
        super({
            name: "BeatmapParser",
            create: native.create_beatmap_parser,
            free: native.free_beatmap_parser,
            parse: native.beatmap_parser_parse,
            write: native.beatmap_parser_write,
            get: native.beatmap_parser_get,
            getByName: native.beatmap_parser_get_by_name,
            update: native.beatmap_parser_update,
            last_error: native.beatmap_parser_last_error
        });
    }
}

export class OsuDbParser extends ParserWithUpdateAndGetByName<OsuLegacyDatabase, OsuDbKey, OsuDbUpdate> {
    constructor() {
        super({
            name: "OsuDbParser",
            create: native.create_osu_db_parser,
            free: native.free_osu_db_parser,
            parse: native.osu_db_parser_parse,
            write: native.osu_db_parser_write,
            get: native.osu_db_parser_get,
            getByName: native.osu_db_parser_get_by_name,
            update: native.osu_db_parser_update,
            last_error: native.osu_db_parser_last_error
        });
    }
}

export class OsuCollectionDbParser extends ParserWithUpdateAndGetByName<
    OsuCollectionDb,
    OsuCollectionDbKey,
    OsuCollectionDbUpdate
> {
    constructor() {
        super({
            name: "OsuCollectionDbParser",
            create: native.create_osu_collection_db_parser,
            free: native.free_osu_collection_db_parser,
            parse: native.osu_collection_db_parser_parse,
            write: native.osu_collection_db_parser_write,
            get: native.osu_collection_db_parser_get,
            getByName: native.osu_collection_db_parser_get_by_name,
            update: native.osu_collection_db_parser_update,
            last_error: native.osu_collection_db_parser_last_error
        });
    }
}

export class OsuScoresDbParser extends ParserBase<OsuScoresDb> {
    constructor() {
        super({
            name: "OsuScoresDbParser",
            create: native.create_osu_scores_db_parser,
            free: native.free_osu_scores_db_parser,
            parse: native.osu_scores_db_parser_parse,
            write: native.osu_scores_db_parser_write,
            get: native.osu_scores_db_parser_get,
            last_error: native.osu_scores_db_parser_last_error
        });
    }
}

export class OsuReplayParser extends ParserBase<OsuReplay> {
    constructor() {
        super({
            name: "OsuReplayParser",
            create: native.create_osu_replay_parser,
            free: native.free_osu_replay_parser,
            parse: native.osu_replay_parser_parse,
            write: native.osu_replay_parser_write,
            get: native.osu_replay_parser_get,
            last_error: native.osu_replay_parser_last_error
        });
    }
}

export class OsdbParser extends ParserWithUpdateAndGetByName<OsdbData, OsdbKey, OsdbUpdate> {
    constructor() {
        super({
            name: "OsdbParser",
            create: native.create_osdb_parser,
            free: native.free_osdb_parser,
            parse: native.osdb_parser_parse,
            write: native.osdb_parser_write,
            get: native.osdb_parser_get,
            getByName: native.osdb_parser_get_by_name,
            update: native.osdb_parser_update,
            last_error: native.osdb_parser_last_error
        });
    }
}

export { native };
export * from "./types/types";
