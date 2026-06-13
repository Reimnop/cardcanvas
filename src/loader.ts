import { WASM_BASE64, type WasmExports } from "./wasm";

export type WasmInstance = WebAssembly.Instance & {
  exports: WasmExports;
  getMemoryView: () => DataView;
};

let instance: WasmInstance | null = null;

export async function getInstance(): Promise<WasmInstance> {
  if (instance) {
    return instance;
  }
  
  const bytes = Uint8Array.fromBase64(WASM_BASE64);
  const result = await WebAssembly.instantiate(bytes as BufferSource);
  instance = result.instance as WasmInstance;
  instance.getMemoryView = function() {
    return new DataView(this.exports.memory.buffer);
  };
  return instance;
}
