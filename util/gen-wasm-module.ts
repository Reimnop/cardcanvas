import { readFileSync, writeFileSync } from "fs";

export function genWasmModule() {
  const wasm = readFileSync("wasm/card.wasm");
  const base64 = Buffer.from(wasm).toString("base64");

  let pos = 8; // skip magic + version

  const readULEB128 = () => {
    let result = 0, shift = 0;
    while (true) {
      const byte = wasm[pos++];
      result |= (byte & 0x7f) << shift;
      if ((byte & 0x80) === 0) break;
      shift += 7;
    }
    return result;
  };

  const readValType = () => {
    const b = wasm[pos++];
    switch (b) {
      case 0x7f: return "number"; // i32
      case 0x7e: return "bigint"; // i64
      case 0x7d: return "number"; // f32
      case 0x7c: return "number"; // f64
      default: return "unknown";
    }
  };

  let types = [], funcTypeIndices = [], exports_ = [], importedFuncCount = 0;

  while (pos < wasm.length) {
    const sectionId = wasm[pos++];
    const sectionSize = readULEB128();
    const sectionEnd = pos + sectionSize;

    if (sectionId === 1) { // Type section
      const count = readULEB128();
      for (let i = 0; i < count; i++) {
        pos++; // 0x60 func type marker
        const paramCount = readULEB128();
        const params = Array.from({ length: paramCount }, readValType);
        const resultCount = readULEB128();
        const results = Array.from({ length: resultCount }, readValType);
        types.push({ params, results });
      }
    } else if (sectionId === 2) { // Import section
      const count = readULEB128();
      for (let i = 0; i < count; i++) {
        const modLen = readULEB128(); pos += modLen;
        const nameLen = readULEB128(); pos += nameLen;
        const kind = wasm[pos++];
        if (kind === 0) { importedFuncCount++; readULEB128(); } // func
        else if (kind === 1) { readULEB128(); wasm[pos++]; wasm[pos++]; } // table
        else if (kind === 2) { wasm[pos++]; readULEB128(); } // memory
        else if (kind === 3) { readULEB128(); pos++; } // global
      }
    } else if (sectionId === 3) { // Function section
      const count = readULEB128();
      for (let i = 0; i < count; i++) funcTypeIndices.push(readULEB128());
    } else if (sectionId === 7) { // Export section
      const count = readULEB128();
      for (let i = 0; i < count; i++) {
        const nameLen = readULEB128();
        const name = new TextDecoder().decode(wasm.slice(pos, pos + nameLen)); pos += nameLen;
        const kind = wasm[pos++];
        const index = readULEB128();
        exports_.push({ name, kind, index });
      }
    } else {
      pos = sectionEnd;
    }
  }

  const lines = [
    `// Auto-generated from wasm/card.wasm`,
    `export const WASM_BASE64 = "${base64}";`,
    ``,
    `export interface WasmExports {`,
    `  memory: WebAssembly.Memory;`
  ];

  for (const { name, kind, index } of exports_) {
    if (name === "memory") continue;
    if (kind === 0) { // function
      const funcIdx = index - importedFuncCount;
      const type = types[funcTypeIndices[funcIdx]];
      const params = type.params.map((t, i) => `arg${i}: ${t}`).join(", ");
      const ret = type.results.length === 0 ? "void" : type.results[0];
      lines.push(`  ${name}: (${params}) => ${ret};`);
    }
  }

  lines.push(`}`);
  writeFileSync("src/wasm.ts", lines.join("\n"));
}
