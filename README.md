# wasm runtime for GoldSrc
This repository is based on [SamVanheer/halflife-updated][1], see its
[README.md][2] for more information.

This repository contains a WASM runtime implementation for the GoldSrc
(Half-Life) engine using [wasm-micro-runtime][3].  
It allows modders and level-designers to load sandboxed executables at runtime,
allowing much more complex scripting than the entity system of the base engine
can do.

Exported native functions are documented in [demo/wasmsrc/lib/native_engine.h](demo/wasmsrc/lib/native_engine.h).

Expected WASM entrypoints are documented in  [demo/wasmsrc/lib/native.h](demo/wasmsrc/lib/native.h).

## Demo
[![Demo](https://raw.githubusercontent.com/L-P/halflife-wasm/media/demo.jpg)](https://raw.githubusercontent.com/L-P/halflife-wasm/media/demo.mp4)

Example maps with WASM sources are available in the [demo](./demo) directory,
they require a nightly build of `clang-18` and `wasm-ld-18` to build. If this
is too much of a hassle and you don't want to build them you can remove
`demo/wasm/Tupfile` and `demo/wasmsrc/Tupfile`.  
Demo maps use the [GoldSrc dev textures WAD][4].

## Building
- On Windows, follow the instructions of the [halflife-updated][1] project, to
  build wasm artifacts, you're on your own.
- On Linux, install [tup](https://gittup.org/tup/) and run:
    1. `$ tup init` once at the root of the repository.
    2. `$ tup` to build. You can also use `$ tup monitor -f -a` to rebuild
       automatically when a file changes.
    3. The built libraries are available at `dlls/hl.so` and
       `cl_dll/client.so`.

For your convenience, the wasmr libraries are prebuilt, if you wish to compile
them yourself you can build the [wasm-micro-runtime][3] using the following
`cmake` invocations and put the resulting files in [lib/public](lib/public):

```
# win32
$ cmake .. \
    -A Win32 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_GENERATOR_PLATFORM=Win32 \
    -DWAMR_BUILD_PLATFORM=windows \
    -DWAMR_BUILD_TARGET=X86_32 \
    -DWAMR_BUILD_FAST_INTERP=0 \
    -DWAMR_BUILD_SIMD=0 \
    -DWAMR_BUILD_LIBC_BUILTIN=0 \
    -DWAMR_BUILD_LIBC_WASI=0 \
    -DWAMR_BUILD_AOT=0

# linux
$ cmake ../CMakeLists.txt \
    -DCMAKE_BUILD_TYPE=Release \
    -DWAMR_BUILD_PLATFORM=linux \
    -DWAMR_BUILD_TARGET=X86_32 \
    -DWAMR_BUILD_FAST_INTERP=0 \
    -DWAMR_BUILD_SIMD=0 \
    -DWAMR_BUILD_LIBC_BUILTIN=0 \
    -DWAMR_BUILD_LIBC_WASI=0 \
    -DWAMR_BUILD_AOT=0
```

## Unstable API warning
The native functions currently exposed to the WASM runtime and the WASM runtime
calling conventions **are not to be considered stable**.
This is an early implementation where things are implemented and refactored as
I need them for a mod that is currently in development.

[1]: https://github.com/SamVanheer/halflife-updated
[2]: https://github.com/SamVanheer/halflife-updated/blob/master/README.md
[3]: https://github.com/bytecodealliance/wasm-micro-runtime
[4]: https://gamebanana.com/mods/6694
