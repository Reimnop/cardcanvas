export type Pointer = number;

export interface NativeObject {
  ptr: Pointer;
  free(): void;
}