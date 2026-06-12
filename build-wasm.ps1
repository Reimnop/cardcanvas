#Requires -Version 7
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$WasiSdk = if ($env:WASI_SDK) { $env:WASI_SDK } else { 'C:\wasi-sdk' }

if (-not (Test-Path $WasiSdk -PathType Container)) {
    Write-Error "wasi-sdk not found at $WasiSdk"
    Write-Host "Install it from https://github.com/WebAssembly/wasi-sdk/releases"
    Write-Host "or set the WASI_SDK environment variable to its location"
    exit 1
}

New-Item -ItemType Directory -Force -Path wasm | Out-Null

& "$WasiSdk\bin\clang.exe" `
    --target=wasm32 `
    -O2 `
    -fno-builtin `
    -nostdlib `
    "-Wl,--no-entry" `
    "-Wl,--export-all" `
    "-Wl,--strip-all" `
    -o wasm/card.wasm `
    native/encoding.c `
    native/glyph_table.c `
    native/canvas.c `
    native/color.c `
    native/walloc.c

if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$Size = (Get-Item wasm/card.wasm).Length
Write-Host "Built wasm/card.wasm ($Size bytes)"

Write-Host "Generating TypeScript module for the WASM module"
& npm run gen-wasm-module
