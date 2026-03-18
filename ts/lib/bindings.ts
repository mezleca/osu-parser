import node_gyp_build from "node-gyp-build";
import path from "path";

import type { NativeBindings } from "../types/types";

const native = node_gyp_build(path.join(__dirname, "..", "..")) as NativeBindings;

export { native };
