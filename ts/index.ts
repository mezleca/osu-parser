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

const resolve_fns = (prefix: string) => {
    const n = native as Record<string, any>;

    return {
        create: n[`create_${prefix}`] as () => bigint,
        free: n[`free_${prefix}`] as (handle: bigint) => void,
        parse: n[`${prefix}_parse`] as (handle: bigint, location: string) => Promise<boolean>,
        write: n[`${prefix}_write`] as (handle: bigint) => Promise<boolean>,
        get: n[`${prefix}_get`] as (handle: bigint) => unknown,
        last_error: n[`${prefix}_last_error`] as (handle: bigint) => string | null,
        get_by_name: n[`${prefix}_get_by_name`] as ((handle: bigint, key: string) => unknown) | undefined,
        update: n[`${prefix}_update`] as ((handle: bigint, patch: unknown) => boolean) | undefined
    };
};

class Parser<TGet> {
    protected handle: bigint;
    protected prefix: string;
    protected fns: ReturnType<typeof resolve_fns>;

    constructor(prefix: string) {
        this.prefix = prefix;
        this.fns = resolve_fns(prefix);
        this.handle = this.fns.create();
    }

    get ptr(): bigint {
        return this.handle;
    }

    async parse(location: string): Promise<this> {
        this.assert_handle();
        const ok = await this.fns.parse(this.handle, location);

        if (!ok) {
            const message = this.fns.last_error(this.handle);
            throw new Error(message ? `${this.prefix}.parse failed: ${message}` : `${this.prefix}.parse failed`);
        }

        return this;
    }

    async write(): Promise<void> {
        this.assert_handle();
        const ok = await this.fns.write(this.handle);

        if (!ok) {
            throw new Error(`${this.prefix}.write failed`);
        }
    }

    get(): TGet {
        this.assert_handle();
        return this.fns.get(this.handle) as TGet;
    }

    last_error(): string | null {
        this.assert_handle();
        return this.fns.last_error(this.handle);
    }

    free(): void {
        if (this.handle !== 0n) {
            this.fns.free(this.handle);
            this.handle = 0n;
        }
    }

    protected assert_handle() {
        if (this.handle === 0n) {
            throw new Error("parser handle is not valid");
        }
    }
}

class UpdatableParser<TGet extends object, TKey extends keyof TGet & string, TUpdate> extends Parser<TGet> {
    constructor(prefix: string) {
        super(prefix);
    }

    get_by_name<K extends TKey>(key: K): TGet[K] {
        this.assert_handle();
        if (!this.fns.get_by_name) {
            throw new Error(`${this.prefix}.get_by_name not implemented`);
        }
        return this.fns.get_by_name(this.handle, key) as TGet[K];
    }

    update(patch: TUpdate): this {
        this.assert_handle();
        if (!this.fns.update) {
            throw new Error(`${this.prefix}.update not implemented`);
        }
        const ok = this.fns.update(this.handle, patch);
        if (!ok) {
            const message = this.fns.last_error(this.handle);
            throw new Error(message ? `${this.prefix}.update failed: ${message}` : `${this.prefix}.update failed`);
        }
        return this;
    }
}

export class BeatmapParser extends UpdatableParser<OsuFileFormat, BeatmapKey, BeatmapUpdate> {
    constructor() {
        super("beatmap_parser");
    }
}

export class OsuDbParser extends UpdatableParser<OsuLegacyDatabase, OsuDbKey, OsuDbUpdate> {
    constructor() {
        super("osu_db_parser");
    }
}

export class OsuCollectionDbParser extends UpdatableParser<OsuCollectionDb, OsuCollectionDbKey, OsuCollectionDbUpdate> {
    constructor() {
        super("osu_collection_db_parser");
    }
}

export class OsuScoresDbParser extends Parser<OsuScoresDb> {
    constructor() {
        super("osu_scores_db_parser");
    }
}

export class OsuReplayParser extends Parser<OsuReplay> {
    constructor() {
        super("osu_replay_parser");
    }
}

export class OsdbParser extends UpdatableParser<OsdbData, OsdbKey, OsdbUpdate> {
    constructor() {
        super("osdb_parser");
    }
}

export { native };
export * from "./types/types";
