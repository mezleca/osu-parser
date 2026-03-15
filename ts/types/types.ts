export interface NativeBindings {
    create_beatmap_parser(): bigint;
    free_beatmap_parser(handle: bigint): void;
    beatmap_parser_parse(handle: bigint, location: string): boolean;
    beatmap_parser_write(handle: bigint): boolean;

    create_osu_db_parser(): bigint;
    free_osu_db_parser(handle: bigint): void;
    osu_db_parser_parse(handle: bigint, location: string): boolean;
    osu_db_parser_write(handle: bigint): boolean;

    create_osu_collection_db_parser(): bigint;
    free_osu_collection_db_parser(handle: bigint): void;
    osu_collection_db_parser_parse(handle: bigint, location: string): boolean;
    osu_collection_db_parser_write(handle: bigint): boolean;

    create_osu_scores_db_parser(): bigint;
    free_osu_scores_db_parser(handle: bigint): void;
    osu_scores_db_parser_parse(handle: bigint, location: string): boolean;
    osu_scores_db_parser_write(handle: bigint): boolean;

    create_osu_replay_parser(): bigint;
    free_osu_replay_parser(handle: bigint): void;
    osu_replay_parser_parse(handle: bigint, location: string): boolean;
    osu_replay_parser_write(handle: bigint): boolean;

    create_osdb_parser(): bigint;
    free_osdb_parser(handle: bigint): void;
    osdb_parser_parse(handle: bigint, location: string): boolean;
    osdb_parser_write(handle: bigint): boolean;
}
