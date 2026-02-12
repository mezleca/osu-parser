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

export interface OsuInput {
    data: Uint8Array;
    id?: string;
}

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
    version: number;
    General: GeneralSection;
    Editor: EditorSection;
    Metadata: MetadataSection;
    Difficulty: DifficultySection;
    Events: EventsSection;
    TimingPoints: TimingPoint[];
    Colours: ColourSection;
    HitObjects: HitObject[];
}

export interface INativeAsyncExporter {
    get_property(data: Uint8Array, key: string): Promise<string>;
    get_properties(data: Uint8Array, keys: string[]): Promise<Record<string, string>>;
    get_section(data: Uint8Array, section: string): Promise<string[]>;
    parse(data: Uint8Array): Promise<OsuFileFormat>;
}
