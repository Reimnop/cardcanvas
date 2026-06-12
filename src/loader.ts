import { WASM_BASE64, type WasmInstance } from "./wasm";

let instance: WasmInstance | null = null;

export async function getInstance(): Promise<WasmInstance> {
  if (instance) {
    return instance;
  }
  
  const bytes = Uint8Array.fromBase64(WASM_BASE64);
  const result = await WebAssembly.instantiate(bytes as BufferSource);
  instance = result.instance as WasmInstance;
  return instance;
}
