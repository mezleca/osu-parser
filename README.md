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
import { BeatmapParser } from "@rel-packages/osu-parser-core";

const parser = new BeatmapParser();
parser.parse("path/to/file.osu");

const data = parser.get();
parser.free();
```
