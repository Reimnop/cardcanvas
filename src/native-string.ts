import type { WasmInstance } from "./loader";
import type { Pointer } from "./native-object";

export interface NativeString {
  ptr: Pointer;
  size: number;
}

let encoder: TextEncoder | null = null;

export function allocNativeString(wasm: WasmInstance, str: string): NativeString {
  if (!encoder) {
    encoder = new TextEncoder();
  }

  const bytes = encoder.encode(str);
  const ptr = wasm.exports.malloc(bytes.length);
  new Uint8Array(wasm.exports.memory.buffer, ptr, bytes.length).set(bytes);
  return { ptr, size: bytes.length };
}

export function freeNativeString(wasm: WasmInstance, nativeStr: NativeString): void {
  wasm.exports.free(nativeStr.ptr);
}
