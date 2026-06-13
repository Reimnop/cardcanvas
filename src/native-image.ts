import { getInstance, type WasmInstance } from "./loader";
import { type Color, packColor, unpackColor } from "./color";
import type { NativeObject, Pointer } from "./native-object";
import { registerNativeObject, releaseNativeObject } from "./memory-manager";

export class NativeImage implements NativeObject {
  ptr: Pointer;
  readonly width: number;
  readonly height: number;

  private wasm: WasmInstance;

  private constructor(ptr: number, wasm: WasmInstance, width: number, height: number) {
    this.ptr = ptr;
    this.wasm = wasm;
    this.width = width;
    this.height = height;

    registerNativeObject(this.wasm, this);
  }

  static async create(width: number, height: number): Promise<NativeImage> {
    const wasm = await getInstance();
    const size = width * height * 4;
    const ptr = wasm.exports.malloc(size);
    return new NativeImage(ptr, wasm, width, height);
  }

  getPixel(x: number, y: number): Color {
    const packed = this.wasm.getMemoryView().getUint32(this.ptr + (y * this.width + x) * 4, true);
    return unpackColor(packed);
  }

  setPixel(x: number, y: number, color: Color): void {
    const packed = packColor(color);
    this.wasm.getMemoryView().setUint32(this.ptr + (y * this.width + x) * 4, packed, true);
  }

  asUint32Array(): Uint32Array {
    return new Uint32Array(this.wasm.exports.memory.buffer, this.ptr, this.width * this.height);
  }

  free(): void {
    releaseNativeObject(this.wasm, this);
  }
}