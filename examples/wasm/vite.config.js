import { defineConfig } from "vite";
import { svelte } from "@sveltejs/vite-plugin-svelte";
import path from "node:path";
import { fileURLToPath } from "node:url";

const CURRENT_DIR = path.dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = path.resolve(CURRENT_DIR, "..", "..");

export default defineConfig({
    plugins: [svelte()],
    server: {
        fs: {
            allow: [REPO_ROOT]
        }
    }
});
