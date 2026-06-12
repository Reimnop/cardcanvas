import type { NativeObject, Pointer } from "./native-object";
import type { WasmInstance } from "./wasm";

let finalizationRegistry: FinalizationRegistry<Pointer> | null = null;

function getFinalizationRegistry(wasm: WasmInstance): FinalizationRegistry<Pointer> {
  if (!finalizationRegistry) {
    finalizationRegistry = new FinalizationRegistry((ptr) => {
      wasm.exports.free(ptr);
    });
  }
  return finalizationRegistry;
}

export function registerNativeObject(wasm: WasmInstance, obj: NativeObject) {
  const registry = getFinalizationRegistry(wasm);
  registry.register(obj, obj.ptr, obj);
}

export function releaseNativeObject(wasm: WasmInstance, obj: NativeObject) {
  if (obj.ptr === 0) {
    return;
  }
  
  wasm.exports.free(obj.ptr);
  const registry = getFinalizationRegistry(wasm);
  registry.unregister(obj);
  obj.ptr = 0; // prevent double free
}