import fs from "fs";
import path from "path";
import { $ } from "bun";

const TARGET_DIR = "build";

const execute_raw = async (bin_name: string, arg_list: string[]) => {
    const full_cmd = `${bin_name} ${arg_list.join(" ")}`;
    console.log(`\nexecuting: ${full_cmd}`);

    try {
        await $`${bin_name} ${arg_list}`;
    } catch {
        process.exit(1);
    }
};

const remove_dir = (dir_path: string) => {
    if (fs.existsSync(dir_path)) {
        try {
            fs.rmSync(dir_path, { recursive: true, force: true });
        } catch (e: any) {
            console.warn(`[warn] could not remove ${dir_path}: ${e.message}`);
        }
    }
};

const compile_native = async () => {
    const TMP_NATIVE = path.join(TARGET_DIR, "native-tmp");

    remove_dir(TMP_NATIVE);
    await execute_raw("cmake-js", ["build", "-G", "Ninja", "--out", TMP_NATIVE]);

    const BIN_NAMES = [path.join(TMP_NATIVE, "osu-beatmap-parser.node"), path.join(TMP_NATIVE, "Release", "osu-beatmap-parser.node")];

    for (const bin_file of BIN_NAMES) {
        if (fs.existsSync(bin_file)) {
            // copy to build/
            fs.copyFileSync(bin_file, path.join(TARGET_DIR, "osu-beatmap-parser.node"));

            // copy to prebuilds for node-gyp-build
            const platform = process.platform;
            const arch = process.arch;
            const prebuilds_dir = path.join("prebuilds", `${platform}-${arch}`);

            if (!fs.existsSync(prebuilds_dir)) {
                fs.mkdirSync(prebuilds_dir, { recursive: true });
            }

            fs.copyFileSync(bin_file, path.join(prebuilds_dir, "osu-beatmap-parser.node"));

            console.log(`\ncopied binary to ${prebuilds_dir}`);
            return;
        }
    }

    process.exit(1);
};

const compile_wasm = async () => {
    const emscripten_bin = process.env.EMSCRIPTEN_BIN || process.env.EMSCRIPTEN || "";
    let emcmake_cmd = emscripten_bin ? path.join(emscripten_bin, "emcmake") : "";

    if (!emcmake_cmd) {
        try {
            const which_res = await $`which emcmake`.text();
            if (which_res) {
                emcmake_cmd = which_res.trim();
            }
        } catch {
            emcmake_cmd = "";
        }
    }

    if (!emcmake_cmd) {
        console.error("emcmake not found. install emscripten or set EMSCRIPTEN_BIN to the emscripten bin directory (example: /usr/lib/emscripten).");
        process.exit(1);
    }

    const TMP_WASM = path.join(TARGET_DIR, "wasm-tmp");
    remove_dir(TMP_WASM);

    await execute_raw(emcmake_cmd, ["cmake", "-S", ".", "-B", TMP_WASM, "-G", "Ninja"]);
    await execute_raw("cmake", ["--build", TMP_WASM]);

    const emscripten_js = path.join(TMP_WASM, "osu-beatmap-parser.js");

    if (!fs.existsSync(emscripten_js)) {
        console.error("emscripten build failed - no output file");
        process.exit(1);
    }

    fs.copyFileSync(emscripten_js, path.join(TARGET_DIR, "osu-beatmap-parser.js"));

    // bundle wrapper with bun
    console.log("\nbundling wasm wrapper...");

    await execute_raw("bun", [
        "build",
        "src/browser/wasm-wrapper.ts",
        "--outfile",
        path.join(TARGET_DIR, "wasm-wrapper.js"),
        "--target",
        "browser",
        "--format",
        "esm",
        "--minify"
    ]);
    await execute_raw("bun", [
        "build",
        "src/browser/wasm-worker.ts",
        "--outfile",
        path.join(TARGET_DIR, "wasm-worker.js"),
        "--target",
        "browser",
        "--format",
        "esm",
        "--minify"
    ]);

    const wrapper_module_bundle = path.join(TARGET_DIR, "wasm-wrapper.js");
    const worker_module_bundle = path.join(TARGET_DIR, "wasm-worker.js");
    if (!fs.existsSync(wrapper_module_bundle)) {
        console.error("wrapper module bundle failed");
        process.exit(1);
    }
    if (!fs.existsSync(worker_module_bundle)) {
        console.error("worker module bundle failed");
        process.exit(1);
    }

    // load content
    let emscripten_code = fs.readFileSync(emscripten_js, "utf-8");
    emscripten_code = emscripten_code.replace(/\n\/\/# sourceMappingURL=.*$/g, "");
    const wrapper_code = fs.readFileSync(wrapper_module_bundle, "utf-8");

    const final_bundle = `${emscripten_code};${wrapper_code};`;

    fs.writeFileSync(path.join(TARGET_DIR, "osu-parser.browser.js"), final_bundle);
    const dist_browser = path.join("dist", "browser");
    if (!fs.existsSync(dist_browser)) {
        fs.mkdirSync(dist_browser, { recursive: true });
    }
    fs.copyFileSync(wrapper_module_bundle, path.join(dist_browser, "wasm-wrapper.js"));
    fs.copyFileSync(worker_module_bundle, path.join(dist_browser, "wasm-worker.js"));
    fs.copyFileSync(emscripten_js, path.join(dist_browser, "osu-beatmap-parser.js"));
    fs.copyFileSync(path.join("src", "browser", "wasm-wrapper.d.ts"), path.join(dist_browser, "wasm-wrapper.d.ts"));

    console.log("\nwasm bundle created: build/osu-parser.browser.js");
};

const main = async () => {
    const EXEC_ARGS = process.argv.slice(2);
    const BUILD_GOAL = EXEC_ARGS[0] || "native";

    if (fs.existsSync(TARGET_DIR) == false) {
        fs.mkdirSync(TARGET_DIR);
    }

    if (BUILD_GOAL == "native") {
        await compile_native();
    } else if (BUILD_GOAL == "wasm") {
        await compile_wasm();
    } else if (BUILD_GOAL == "all") {
        await compile_native();
        await compile_wasm();
    } else if (BUILD_GOAL == "clean") {
        remove_dir(TARGET_DIR);
    } else {
        process.exit(1);
    }
};

await main();
