import { execFileSync } from "child_process";
import { existsSync, mkdirSync, statSync } from "fs";
import { join } from "path";
import { genWasmModule } from "./gen-wasm-module";

const defaultWasiSdk = process.platform === "win32" ? "C:\\wasi-sdk" : "/opt/wasi-sdk";
const wasiSdk = process.env.WASI_SDK ?? defaultWasiSdk;

if (!existsSync(wasiSdk)) {
  console.error(`wasi-sdk not found at ${wasiSdk}`);
  console.error("Install it from https://github.com/WebAssembly/wasi-sdk/releases");
  console.error("or set the WASI_SDK environment variable to its location");
  process.exit(1);
}

mkdirSync("wasm", { recursive: true });

const clang = join(wasiSdk, "bin", "clang");

const sources = [
  "native/encoding.c",
  "native/glyph_table.c",
  "native/canvas.c",
  "native/color.c",
  "native/walloc.c",
];

execFileSync(clang, [
  "--target=wasm32",
  "-Oz",
  "-fno-builtin",
  "-nostdlib",
  "-Wl,--no-entry",
  "-Wl,--export-all",
  "-Wl,--strip-all",
  "-o", "wasm/card.wasm",
  ...sources
], { stdio: "inherit" });

const bytes = statSync("wasm/card.wasm").size;
console.log(`Built wasm/card.wasm (${bytes} bytes)`);

console.log("Generating TypeScript module for the WASM module");
genWasmModule();
