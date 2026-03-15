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

    const BIN_NAMES = [path.join(TMP_NATIVE, "osu-parser.node"), path.join(TMP_NATIVE, "Release", "osu-parser.node")];

    for (const bin_file of BIN_NAMES) {
        if (fs.existsSync(bin_file)) {
            fs.copyFileSync(bin_file, path.join(TARGET_DIR, "osu-parser.node"));

            const platform = process.platform;
            const arch = process.arch;
            const prebuilds_dir = path.join("prebuilds", `${platform}-${arch}`);

            if (!fs.existsSync(prebuilds_dir)) {
                fs.mkdirSync(prebuilds_dir, { recursive: true });
            }

            fs.copyFileSync(bin_file, path.join(prebuilds_dir, "osu-parser.node"));

            console.log(`\ncopied binary to ${prebuilds_dir}`);
            return;
        }
    }

    process.exit(1);
};

const main = async () => {
    const EXEC_ARGS = process.argv.slice(2);
    const BUILD_GOAL = EXEC_ARGS[0] || "native";

    if (fs.existsSync(TARGET_DIR) == false) {
        fs.mkdirSync(TARGET_DIR);
    }

    if (BUILD_GOAL == "native") {
        await compile_native();
    } else if (BUILD_GOAL == "clean") {
        remove_dir(TARGET_DIR);
    } else {
        process.exit(1);
    }
};

await main();
