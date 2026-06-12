import type { WasmInstance } from "./wasm";
import type { NativeImage } from "./native-image";
import { type Color, packColor } from "./color";
import { allocNativeString, freeNativeString } from "./native-string";
import { getInstance } from "./loader";
import type { NativeObject, Pointer } from "./native-object";
import { registerNativeObject, releaseNativeObject } from "./memory-manager";

export class Canvas implements NativeObject {
  ptr: Pointer;

  private wasm: WasmInstance;
  private image: NativeImage;

  private constructor(ptr: number, wasm: WasmInstance, image: NativeImage) {
    this.ptr = ptr;
    this.wasm = wasm;
    this.image = image;

    registerNativeObject(wasm, this);
  }

  static async create(image: NativeImage): Promise<Canvas> {
    const wasm = await getInstance();

    // allocate Canvas struct: 3 x uint32 = 12 bytes
    const canvasPtr = wasm.exports.malloc(12);
    const pxPerCu = image.width / 256;

    wasm.exports.canvas_init(canvasPtr, image.ptr, pxPerCu, image.width);

    return new Canvas(canvasPtr, wasm, image);
  }

  setPixel(xPx: number, yPx: number, color: Color): void {
    this.wasm.exports.canvas_set_pixel(this.ptr, xPx, yPx, packColor(color));
  }

  setBlock(x: number, y: number, color: Color): void {
    this.wasm.exports.canvas_set_block(this.ptr, x, y, packColor(color));
  }

  fillRect(x: number, y: number, width: number, height: number, color: Color): void {
    this.wasm.exports.canvas_fill_rect(this.ptr, x, y, width, height, packColor(color));
  }

  drawGlyph(x: number, y: number, size: number, c: string, color: Color): void {
    this.wasm.exports.canvas_draw_glyph(this.ptr, x, y, size, c.charCodeAt(0), packColor(color));
  }

  drawText(x: number, y: number, size: number, text: string, color: Color): void {
    const str = allocNativeString(this.wasm, text);
    this.wasm.exports.canvas_draw_text(this.ptr, x, y, size, str.ptr, str.size, packColor(color));
    freeNativeString(this.wasm, str);
  }

  drawTextCenterAligned(x: number, y: number, size: number, text: string, color: Color): void {
    const str = allocNativeString(this.wasm, text);
    this.wasm.exports.canvas_draw_text_center_aligned(this.ptr, x, y, size, str.ptr, str.size, packColor(color));
    freeNativeString(this.wasm, str);
  }

  drawTextRightAligned(x: number, y: number, size: number, text: string, color: Color): void {
    const str = allocNativeString(this.wasm, text);
    this.wasm.exports.canvas_draw_text_right_aligned(this.ptr, x, y, size, str.ptr, str.size, packColor(color));
    freeNativeString(this.wasm, str);
  }

  drawSubImage(xPx: number, yPx: number, subImage: NativeImage): void {
    this.wasm.exports.canvas_draw_sub_image(this.ptr, xPx, yPx, subImage.width, subImage.height, subImage.ptr);
  }

  calculateTextRectSize(length: number, size: number): { width: number; height: number } {
    // allocate two uint32s for out params
    const outPtr = this.wasm.exports.malloc(8);
    this.wasm.exports.canvas_calculate_text_rect_size(length, size, outPtr, outPtr + 4);
    const view = new Uint32Array(this.wasm.exports.memory.buffer, outPtr, 2);
    const result = { width: view[0], height: view[1] };
    this.wasm.exports.free(outPtr);
    return result;
  }

  free(): void {
    releaseNativeObject(this.wasm, this);
  }
}