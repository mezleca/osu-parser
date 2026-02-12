<script>
    import { onMount } from "svelte";
    import * as beatmap_parser from "@rel-packages/osu-beatmap-parser/browser";

    let status_text = "initializing...";
    let status_color = "#ffffff";
    let results = [];

    const set_status = (text, color = "#ffffff") => {
        status_text = text;
        status_color = color;
    };

    const add_result = (value, is_section = false) => {
        results = [...results, { value, is_section }];
    };

    const parse_file = async (name) => {
        const response = await fetch(`/${name}`);
        const buffer = await response.arrayBuffer();
        const data = new Uint8Array(buffer);

        const title = await beatmap_parser.get_property(data, "Title");
        const artist = await beatmap_parser.get_property(data, "Artist");
        const parsed = await beatmap_parser.parse(data);

        add_result(`${name}: ${title} - ${artist}`, true);
        add_result(JSON.stringify(parsed, null, 2));
    };

    onMount(async () => {
        try {
            await beatmap_parser.init_wasm();

            set_status("wasm loaded", "#00ff88");
            await parse_file("1.osu");
            await parse_file("2.osu");
            set_status("done", "#00ff88");
        } catch (error) {
            set_status(error.message || String(error), "#ff4d4f");
        }
    });
</script>

<main>
    <div class="status" style={`color: ${status_color}`}>{status_text}</div>

    <div class="results">
        {#each results as item}
            <pre class:item_section={item.is_section}>{item.value}</pre>
        {/each}
    </div>
</main>

<style>
    :global(body) {
        margin: 0;
        padding: 20px;
        font-family: sans-serif;
        background: #242424;
        color: #eee;
    }

    .status {
        font-weight: 700;
        margin-bottom: 10px;
    }

    .results pre {
        margin-top: 10px;
        padding: 10px;
        border: 1px solid #444;
        border-radius: 4px;
        background: #242424;
        white-space: pre-wrap;
        font-family: monospace;
    }

    .results pre.item_section {
        color: #00d9ff;
        margin-top: 15px;
    }
</style>
