# osu-parser

osu! parser library for Node.js.

## required

- Node.js 24+
- Bun (latest)
- CMake + Ninja

## setup

1. `git submodule update --init --recursive`
2. `bun install`
3. `bun run build`

## usage

```ts
import { BeatmapParser } from "osu-parser";

const parser = new BeatmapParser();

const main = async () => {
    try {
        await parser.parse("path/to/file.osu");
        const data = await parser.get();
        console.log(data);
    } catch (error) {
        console.error("parse failed:", parser.last_error() ?? error);
    } finally {
        await parser.free();
    }
};

main();
```
