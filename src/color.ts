export interface Color {
  r: number;
  g: number;
  b: number;
  a: number;
}

export function unpackColor(packed: number): Color {
  return {
    r: packed & 0xFF,
    g: (packed >>> 8) & 0xFF,
    b: (packed >>> 16) & 0xFF,
    a: (packed >>> 24) & 0xFF,
  };
}

export function packColor(color: Color): number {
  return (color.r & 0xFF) | ((color.g & 0xFF) << 8) | ((color.b & 0xFF) << 16) | ((color.a & 0xFF) << 24);
}