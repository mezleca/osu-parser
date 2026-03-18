export type OsuKey =
    | "AudioFilename"
    | "AudioLeadIn"
    | "AudioHash"
    | "PreviewTime"
    | "Countdown"
    | "SampleSet"
    | "StackLeniency"
    | "Mode"
    | "LetterboxInBreaks"
    | "StoryFireInFront"
    | "UseSkinSprites"
    | "AlwaysShowPlayfield"
    | "OverlayPosition"
    | "SkinPreference"
    | "EpilepsyWarning"
    | "CountdownOffset"
    | "SpecialStyle"
    | "WidescreenStoryboard"
    | "SamplesMatchPlaybackRate"
    | "Bookmarks"
    | "DistanceSpacing"
    | "BeatDivisor"
    | "GridSize"
    | "TimelineZoom"
    | "Title"
    | "TitleUnicode"
    | "Artist"
    | "ArtistUnicode"
    | "Creator"
    | "Version"
    | "Source"
    | "Tags"
    | "BeatmapID"
    | "BeatmapSetID"
    | "HPDrainRate"
    | "CircleSize"
    | "OverallDifficulty"
    | "ApproachRate"
    | "SliderMultiplier"
    | "SliderTickRate"
    | "Background"
    | "Video"
    | "Storyboard";

export interface GeneralSection {
    AudioFilename: string;
    AudioLeadIn: number;
    AudioHash: string;
    PreviewTime: number;
    Countdown: number;
    SampleSet: string;
    StackLeniency: number;
    Mode: number;
    LetterboxInBreaks: number;
    StoryFireInFront: number;
    UseSkinSprites: number;
    AlwaysShowPlayfield: number;
    OverlayPosition: string;
    SkinPreference: string;
    EpilepsyWarning: number;
    CountdownOffset: number;
    SpecialStyle: number;
    WidescreenStoryboard: number;
    SamplesMatchPlaybackRate: number;
}

export interface EditorSection {
    Bookmarks: number[];
    DistanceSpacing: number;
    BeatDivisor: number;
    GridSize: number;
    TimelineZoom: number;
}

export interface MetadataSection {
    Title: string;
    TitleUnicode: string;
    Artist: string;
    ArtistUnicode: string;
    Creator: string;
    Version: string;
    Source: string;
    Tags: string;
    BeatmapID: number;
    BeatmapSetID: number;
}

export interface DifficultySection {
    HPDrainRate: number;
    CircleSize: number;
    OverallDifficulty: number;
    ApproachRate: number;
    SliderMultiplier: number;
    SliderTickRate: number;
}

export interface EventBackground {
    filename: string;
    xOffset: number;
    yOffset: number;
}

export interface EventVideo {
    filename: string;
    startTime: number;
    xOffset: number;
    yOffset: number;
}

export interface EventBreak {
    startTime: number;
    endTime: number;
}

export interface EventsSection {
    background: EventBackground | null;
    video: EventVideo | null;
    breaks: EventBreak[];
}

export interface TimingPoint {
    time: number;
    beatLength: number;
    meter: number;
    sampleSet: number;
    sampleIndex: number;
    volume: number;
    uninherited: number;
    effects: number;
}

export interface ColourSection {
    Combos: [number, number, number][];
    SliderTrackOverride: [number, number, number] | null;
    SliderBorder: [number, number, number] | null;
}

export interface HitSample {
    normalSet: number;
    additionSet: number;
    index: number;
    volume: number;
    filename: string;
}

export interface CurvePoint {
    x: number;
    y: number;
}

export interface EdgeSet {
    normalSet: number;
    additionSet: number;
}

export interface HitObject {
    x: number;
    y: number;
    time: number;
    type: number;
    hitSound: number;
    hitSample: HitSample;
    curveType: string;
    curvePoints: CurvePoint[];
    slides: number;
    length: number;
    edgeSounds: number[];
    edgeSets: EdgeSet[];
    endTime: number;
}

export interface OsuFileFormat {
    version: string;
    General: GeneralSection;
    Editor: EditorSection;
    Metadata: MetadataSection;
    Difficulty: DifficultySection;
    Events: EventsSection;
    TimingPoints: TimingPoint[];
    Colours: ColourSection;
    HitObjects: HitObject[];
}

export interface OsuIntFloatPair {
    mod_combination: number;
    star_rating: number;
}

export interface OsuDbTimingPoint {
    bpm: number;
    offset: number;
    inherited: number;
}

export interface OsuDbBeatmap {
    entry_size: number | null;
    artist: string;
    artist_unicode: string;
    title: string;
    title_unicode: string;
    creator: string;
    difficulty: string;
    audio_file_name: string;
    md5: string;
    osu_file_name: string;
    ranked_status: number;
    hitcircle: number;
    sliders: number;
    spinners: number;
    last_modification_time: number;
    approach_rate: number;
    circle_size: number;
    hp_drain: number;
    overall_difficulty: number;
    slider_velocity: number;
    star_rating_standard: OsuIntFloatPair[];
    star_rating_taiko: OsuIntFloatPair[];
    star_rating_ctb: OsuIntFloatPair[];
    star_rating_mania: OsuIntFloatPair[];
    drain_time: number;
    total_time: number;
    audio_preview_time: number;
    timing_points: OsuDbTimingPoint[];
    difficulty_id: number;
    beatmap_id: number;
    thread_id: number;
    grade_standard: number;
    grade_taiko: number;
    grade_ctb: number;
    grade_mania: number;
    local_offset: number;
    stack_leniency: number;
    mode: number;
    source: string;
    tags: string;
    online_offset: number;
    title_font: string;
    unplayed: number;
    last_played: number;
    is_osz2: number;
    folder_name: string;
    last_checked: number;
    ignore_sounds: number;
    ignore_skin: number;
    disable_storyboard: number;
    disable_video: number;
    visual_override: number;
    unknown: number | null;
    last_modified: number;
    mania_scroll_speed: number;
}

export interface OsuLegacyDatabase {
    version: number;
    folder_count: number;
    account_unlocked: number;
    account_unlock_time: number;
    player_name: string;
    beatmaps_count: number;
    beatmaps: OsuDbBeatmap[];
    permissions: number;
}

export interface OsuCollection {
    name: string;
    beatmaps_count: number;
    beatmap_md5: string[];
}

export interface OsuCollectionDb {
    version: number;
    collections_count: number;
    collections: OsuCollection[];
}

export interface OsuScore {
    mode: number;
    version: number;
    beatmap_md5: string;
    player_name: string;
    replay_md5: string;
    count_300: number;
    count_100: number;
    count_50: number;
    count_geki: number;
    count_katu: number;
    count_miss: number;
    score: number;
    max_combo: number;
    perfect: number;
    mods: number;
    life_bar_graph: string;
    timestamp: number;
    replay_data_length: number;
    replay_data: Uint8Array;
    online_score_id: number;
    additional_mod_info: number | null;
}

export interface OsuScoresBeatmap {
    beatmap_md5: string;
    scores_count: number;
    scores: OsuScore[];
}

export interface OsuScoresDb {
    version: number;
    beatmaps_count: number;
    beatmaps: OsuScoresBeatmap[];
}

export interface OsuReplay {
    mode: number;
    version: number;
    beatmap_md5: string;
    player_name: string;
    replay_md5: string;
    count_300: number;
    count_100: number;
    count_50: number;
    count_geki: number;
    count_katu: number;
    count_miss: number;
    score: number;
    max_combo: number;
    perfect: number;
    mods: number;
    life_bar_graph: string;
    timestamp: number;
    replay_data_length: number;
    replay_data: Uint8Array;
    online_score_id: number;
    additional_mod_info: number | null;
}

export interface OsdbBeatmap {
    difficulty_id: number;
    beatmapset_id: number;
    artist: string;
    title: string;
    difficulty: string;
    checksum: string;
    user_comment: string;
    mode: number;
    difficulty_rating: number;
}

export interface OsdbCollection {
    name: string;
    online_id: number;
    beatmaps: OsdbBeatmap[];
    hash_only_beatmaps: string[];
}

export interface OsdbData {
    save_data: number;
    last_editor: string;
    count: number;
    collections: OsdbCollection[];
}

export interface NativeBindings {
    create_beatmap_parser(): bigint;
    free_beatmap_parser(handle: bigint): void;
    beatmap_parser_parse(handle: bigint, location: string): boolean;
    beatmap_parser_write(handle: bigint): boolean;
    beatmap_parser_last_error(handle: bigint): string | null;
    beatmap_parser_get(handle: bigint): OsuFileFormat;

    create_osu_db_parser(): bigint;
    free_osu_db_parser(handle: bigint): void;
    osu_db_parser_parse(handle: bigint, location: string): boolean;
    osu_db_parser_write(handle: bigint): boolean;
    osu_db_parser_last_error(handle: bigint): string | null;
    osu_db_parser_get(handle: bigint): OsuLegacyDatabase;

    create_osu_collection_db_parser(): bigint;
    free_osu_collection_db_parser(handle: bigint): void;
    osu_collection_db_parser_parse(handle: bigint, location: string): boolean;
    osu_collection_db_parser_write(handle: bigint): boolean;
    osu_collection_db_parser_last_error(handle: bigint): string | null;
    osu_collection_db_parser_get(handle: bigint): OsuCollectionDb;

    create_osu_scores_db_parser(): bigint;
    free_osu_scores_db_parser(handle: bigint): void;
    osu_scores_db_parser_parse(handle: bigint, location: string): boolean;
    osu_scores_db_parser_write(handle: bigint): boolean;
    osu_scores_db_parser_last_error(handle: bigint): string | null;
    osu_scores_db_parser_get(handle: bigint): OsuScoresDb;

    create_osu_replay_parser(): bigint;
    free_osu_replay_parser(handle: bigint): void;
    osu_replay_parser_parse(handle: bigint, location: string): boolean;
    osu_replay_parser_write(handle: bigint): boolean;
    osu_replay_parser_last_error(handle: bigint): string | null;
    osu_replay_parser_get(handle: bigint): OsuReplay;

    create_osdb_parser(): bigint;
    free_osdb_parser(handle: bigint): void;
    osdb_parser_parse(handle: bigint, location: string): boolean;
    osdb_parser_write(handle: bigint): boolean;
    osdb_parser_last_error(handle: bigint): string | null;
    osdb_parser_get(handle: bigint): OsdbData;
}
