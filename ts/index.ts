import { native } from "./lib/bindings";

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
}

export class BeatmapParser extends BaseParser {
    constructor() {
        super(native.create_beatmap_parser());
    }

    parse(location: string): boolean {
        return native.beatmap_parser_parse(this.handle, location);
    }

    write(): boolean {
        return native.beatmap_parser_write(this.handle);
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

    parse(location: string): boolean {
        return native.osu_db_parser_parse(this.handle, location);
    }

    write(): boolean {
        return native.osu_db_parser_write(this.handle);
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

    parse(location: string): boolean {
        return native.osu_collection_db_parser_parse(this.handle, location);
    }

    write(): boolean {
        return native.osu_collection_db_parser_write(this.handle);
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

    parse(location: string): boolean {
        return native.osu_scores_db_parser_parse(this.handle, location);
    }

    write(): boolean {
        return native.osu_scores_db_parser_write(this.handle);
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

    parse(location: string): boolean {
        return native.osu_replay_parser_parse(this.handle, location);
    }

    write(): boolean {
        return native.osu_replay_parser_write(this.handle);
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

    parse(location: string): boolean {
        return native.osdb_parser_parse(this.handle, location);
    }

    write(): boolean {
        return native.osdb_parser_write(this.handle);
    }

    free() {
        native.free_osdb_parser(this.handle);
        this.clear_handle();
    }
}

export { native };
export * from "./types/types";
