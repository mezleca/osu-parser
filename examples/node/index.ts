import fs from "fs";
import path from "path";

import { get_property, get_section, parse } from "../../src/index";

const TEST_LOCATION = process.platform == "linux" ? "/home/rel/.local/share/osu-wine/osu!/Songs/" : "C:\\Users\\rel\\AppData\\Local\\osu!";

const main = async () => {
    console.log("reading files from osu! songs folder...\n");

    const files = fs
        .readdirSync(TEST_LOCATION, { recursive: true, withFileTypes: true })
        .filter((f) => f.isFile())
        .filter((f) => f.name.includes(".osu"));

    const paths = files.map((f) => path.join(f.parentPath, f.name));

    if (paths.length == 0) {
        console.log("no .osu files found");
        return;
    }

    const p = paths[0];
    const data = fs.readFileSync(p);

    console.log("--- single property test ---");
    console.log("file:", p);
    console.log("title:", get_property(data, "Title"));
    console.log("artist:", get_property(data, "Artist"));

    console.log("\n--- get_section test ---");
    const general_lines = get_section(data, "General");
    console.log("general section lines:", general_lines.length);
    general_lines.slice(0, 3).forEach((line) => console.log("  ", line));

    console.log("\n--- full parse test ---");
    const parsed = parse(data);

    console.log("version:", parsed.version);
    console.log("\n[General]");
    console.log("  AudioFilename:", parsed.General.AudioFilename);
    console.log("  Mode:", parsed.General.Mode);
    console.log("  PreviewTime:", parsed.General.PreviewTime);
    console.log("  StackLeniency:", parsed.General.StackLeniency);

    console.log("\n[Editor]");
    console.log("  Bookmarks:", parsed.Editor.Bookmarks.length, "items");
    console.log("  BeatDivisor:", parsed.Editor.BeatDivisor);
    console.log("  DistanceSpacing:", parsed.Editor.DistanceSpacing);

    console.log("\n[Metadata]");
    console.log("  Title:", parsed.Metadata.Title);
    console.log("  TitleUnicode:", parsed.Metadata.TitleUnicode || "(empty)");
    console.log("  Artist:", parsed.Metadata.Artist);
    console.log("  Creator:", parsed.Metadata.Creator);
    console.log("  Version:", parsed.Metadata.Version);
    console.log("  BeatmapID:", parsed.Metadata.BeatmapID);
    console.log("  BeatmapSetID:", parsed.Metadata.BeatmapSetID);

    console.log("\n[Difficulty]");
    console.log("  HP:", parsed.Difficulty.HPDrainRate);
    console.log("  CS:", parsed.Difficulty.CircleSize);
    console.log("  OD:", parsed.Difficulty.OverallDifficulty);
    console.log("  AR:", parsed.Difficulty.ApproachRate);
    console.log("  SliderMultiplier:", parsed.Difficulty.SliderMultiplier);

    console.log("\n[Events]");
    console.log("  background:", parsed.Events.background?.filename || "(none)");
    console.log("  video:", parsed.Events.video?.filename || "(none)");
    console.log("  breaks:", parsed.Events.breaks.length);

    console.log("\n[TimingPoints]");
    console.log("  count:", parsed.TimingPoints.length);
    if (parsed.TimingPoints.length > 0) {
        const first = parsed.TimingPoints[0];
        console.log("  first: time=%d beatLength=%d uninherited=%d", first.time, first.beatLength, first.uninherited);
    }

    console.log("\n[Colours]");
    console.log("  combos:", parsed.Colours.Combos.length);
    parsed.Colours.Combos.forEach((c, i) => {
        console.log("    Combo%d: rgb(%d, %d, %d)", i + 1, c[0], c[1], c[2]);
    });

    console.log("\n[HitObjects]");
    console.log("  count:", parsed.HitObjects.length);
    if (parsed.HitObjects.length > 0) {
        const first = parsed.HitObjects[0];
        const type_flags = [];
        if (first.type & 1) type_flags.push("circle");
        if (first.type & 2) type_flags.push("slider");
        if (first.type & 8) type_flags.push("spinner");
        if (first.type & 128) type_flags.push("hold");

        console.log("  first: pos=(%d,%d) time=%d type=[%s]", first.x, first.y, first.time, type_flags.join(","));

        if (first.type & 2) {
            console.log("    curveType=%s slides=%d length=%d", first.curveType, first.slides, first.length);
        }
    }

    console.log("\n--- done ---");
};

main().catch(console.error);
