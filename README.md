## osu-beatmap-parser

native .osu beatmap parser used on [osu-stuff](https://github.com/mezleca/osu-stuff)

## installation

```bash
npm install @rel-packages/osu-beatmap-parser
```

## usage

check [examples](https://github.com/mezleca/osu-beatmap-parser/tree/main/examples) for wasm / nodejs usage examples.

### node

```ts
import { parse } from "@rel-packages/osu-beatmap-parser";
const parsed = await parser.parse(data);
```

### wasm (browser)

```ts
import { init_wasm, parse } from "@rel-packages/osu-beatmap-parser/browser";

await init_wasm();
const parsed = await parse(data);
```
