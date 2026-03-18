import { native } from "./lib/bindings";
import type {
    BeatmapKey,
    BeatmapUpdate,
    OsdbData,
    OsdbKey,
    OsdbUpdate,
    OsuCollectionDb,
    OsuCollectionDbKey,
    OsuCollectionDbUpdate,
    OsuDbKey,
    OsuDbFilterProperties,
    OsuDbBeatmap,
    OsuDbBeatmapMinimal,
    OsuDbUpdate,
    OsuFileFormat,
    OsuLegacyDatabase,
    OsuReplay,
    OsuDbTimingPoint,
    OsuScoresDb
} from "./types/types";

export const get_common_bpm = (timing_points: OsuDbTimingPoint[], length = 0) => {
    return native.get_common_bpm(timing_points, length);
};

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
        get_header: n[`${prefix}_get_header`] as ((handle: bigint) => unknown) | undefined,
        get_beatmaps_range: n[`${prefix}_get_beatmaps_range`] as
            | ((handle: bigint, start: number, count: number) => unknown)
            | undefined,
        get_by_md5: n[`${prefix}_get_by_md5`] as ((handle: bigint, md5: string) => unknown) | undefined,
        get_minimal_by_md5: n[`${prefix}_get_minimal_by_md5`] as ((handle: bigint, md5: string) => unknown) | undefined,
        get_by_beatmapset_id: n[`${prefix}_get_by_beatmapset_id`] as
            | ((handle: bigint, beatmapset_id: number) => unknown)
            | undefined,
        get_by_difficulty_id: n[`${prefix}_get_by_difficulty_id`] as
            | ((handle: bigint, difficulty_id: number) => unknown)
            | undefined,
        update: n[`${prefix}_update`] as ((handle: bigint, patch: unknown) => boolean) | undefined,
        filter_by_properties: n[`${prefix}_filter_by_properties`] as
            | ((handle: bigint, properties: unknown) => unknown)
            | undefined,
        filter_md5_by_properties: n[`${prefix}_filter_md5_by_properties`] as
            | ((handle: bigint, properties: unknown) => string[])
            | undefined,
        filter_ids_by_properties: n[`${prefix}_filter_ids_by_properties`] as
            | ((handle: bigint, properties: unknown) => number[])
            | undefined,
        update_duration: n[`${prefix}_update_duration`] as ((handle: bigint, updates: unknown) => boolean) | undefined
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

    get_header(): Omit<OsuLegacyDatabase, "beatmaps"> {
        this.assert_handle();
        const fn = (this.fns as any).get_header as ((handle: bigint) => unknown) | undefined;
        if (!fn) {
            throw new Error(`${this.prefix}.get_header not implemented`);
        }
        return fn(this.handle) as Omit<OsuLegacyDatabase, "beatmaps">;
    }

    get_beatmaps_range(start: number, count: number): OsuDbBeatmap[] {
        this.assert_handle();
        const fn = (this.fns as any).get_beatmaps_range as
            | ((handle: bigint, start: number, count: number) => unknown)
            | undefined;
        if (!fn) {
            throw new Error(`${this.prefix}.get_beatmaps_range not implemented`);
        }
        return fn(this.handle, start, count) as OsuDbBeatmap[];
    }

    get_by_md5(md5: string): OsuDbBeatmap | undefined {
        this.assert_handle();
        const fn = (this.fns as any).get_by_md5 as ((handle: bigint, md5: string) => unknown) | undefined;
        if (!fn) {
            throw new Error(`${this.prefix}.get_by_md5 not implemented`);
        }
        return fn(this.handle, md5) as OsuDbBeatmap | undefined;
    }

    get_minimal_by_md5(md5: string) {
        this.assert_handle();
        const fn = (this.fns as any).get_minimal_by_md5 as ((handle: bigint, md5: string) => unknown) | undefined;
        if (!fn) {
            throw new Error(`${this.prefix}.get_minimal_by_md5 not implemented`);
        }
        return fn(this.handle, md5) as OsuDbBeatmapMinimal | undefined;
    }

    get_by_beatmapset_id(beatmapset_id: number): OsuDbBeatmap[] {
        this.assert_handle();
        const fn = (this.fns as any).get_by_beatmapset_id as
            | ((handle: bigint, beatmapset_id: number) => unknown)
            | undefined;
        if (!fn) {
            throw new Error(`${this.prefix}.get_by_beatmapset_id not implemented`);
        }
        return fn(this.handle, beatmapset_id) as OsuDbBeatmap[];
    }

    get_by_difficulty_id(difficulty_id: number): OsuDbBeatmap | undefined {
        this.assert_handle();
        const fn = (this.fns as any).get_by_difficulty_id as
            | ((handle: bigint, difficulty_id: number) => unknown)
            | undefined;
        if (!fn) {
            throw new Error(`${this.prefix}.get_by_difficulty_id not implemented`);
        }
        return fn(this.handle, difficulty_id) as OsuDbBeatmap | undefined;
    }

    filter_by_properties(properties: OsuDbFilterProperties) {
        this.assert_handle();
        if (!this.fns.filter_by_properties) {
            throw new Error(`${this.prefix}.filter_by_properties not implemented`);
        }
        return this.fns.filter_by_properties(this.handle, properties) as OsuDbBeatmap[];
    }

    filter_md5_by_properties(properties: OsuDbFilterProperties): string[] {
        this.assert_handle();
        if (!this.fns.filter_md5_by_properties) {
            throw new Error(`${this.prefix}.filter_md5_by_properties not implemented`);
        }
        return this.fns.filter_md5_by_properties(this.handle, properties);
    }

    filter_ids_by_properties(properties: OsuDbFilterProperties): number[] {
        this.assert_handle();
        if (!this.fns.filter_ids_by_properties) {
            throw new Error(`${this.prefix}.filter_ids_by_properties not implemented`);
        }
        return this.fns.filter_ids_by_properties(this.handle, properties);
    }

    update_duration(updates: { md5: string; duration?: number | null }[]): boolean {
        this.assert_handle();
        const fn = (this.fns as any).update_duration as ((handle: bigint, updates: unknown) => boolean) | undefined;
        if (!fn) {
            throw new Error(`${this.prefix}.update_duration not implemented`);
        }
        return fn(this.handle, updates);
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
