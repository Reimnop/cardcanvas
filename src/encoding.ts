import { getInstance } from "./loader";
import { type NativeImage } from "./native-image";

export async function calculateDimensionPx(pxPerCu: number): Promise<number> {
  const wasm = await getInstance();
  return wasm.exports.encoding_calculate_dimension_px(pxPerCu);
}

export async function calculateTotalCapacity(pxPerCu: number): Promise<number> {
  const wasm = await getInstance();
  return wasm.exports.encoding_calculate_total_capacity(pxPerCu);
}

export async function calculateRequiredPxPerCu(size: number): Promise<number> {
  const wasm = await getInstance();
  return wasm.exports.encoding_calculate_required_px_per_cu(size);
}

export async function encodeCardData(image: NativeImage, pxPerCu: number, data: Uint8Array) {
  const wasm = await getInstance();

  // alloc input buffer
  const inputPtr = wasm.exports.malloc(data.length);
  new Uint8Array(wasm.exports.memory.buffer, inputPtr, data.length).set(data);

  wasm.exports.encoding_encode_card_data(pxPerCu, inputPtr, data.length, image.ptr);

  wasm.exports.free(inputPtr);
}

export async function decodeCardData(image: NativeImage): Promise<Uint8Array> {
  const wasm = await getInstance();

  // alloc output buffer
  const pxPerCu = image.width / 256;
  const capacity = wasm.exports.encoding_calculate_total_capacity(pxPerCu);
  const outputPtr = wasm.exports.malloc(capacity);
  const outputLenPtr = wasm.exports.malloc(4);

  wasm.exports.encoding_decode_card_data(image.ptr, image.width, outputPtr, outputLenPtr);

  const outputLen = new Uint32Array(wasm.exports.memory.buffer, outputLenPtr, 1)[0];
  const result = new Uint8Array(wasm.exports.memory.buffer, outputPtr, outputLen).slice();

  wasm.exports.free(outputPtr);
  wasm.exports.free(outputLenPtr);
  return result;
}
