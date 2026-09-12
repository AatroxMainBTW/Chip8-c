# Chip8-c

A CHIP-8 interpreter written in C, using SDL2 for graphics and input. CHIP-8 is an interpreted virtual machine from 1977 (originally on the COSMAC VIP) — this project implements the fetch–decode–execute loop, the full opcode set, a 64×32 display, and the 16-key hex keypad.

## Why C?

I built this to learn C. I'd already written a CHIP-8 emulator in C++ before, so redoing it in C let me focus on the language itself rather than the problem — the CHIP-8 architecture was familiar, so the new part was C: manual memory layout, structs and pointers, `memset`/`fread`, no classes or STL to lean on. Reimplementing something I already understood was a deliberate way to get comfortable with C's lower-level style.


## Building

Requires **CMake** (3.16+) and **SDL2**.

### macOS
```
brew install sdl2
cmake -S . -B build
cmake --build build
```

### Windows (Visual Studio + vcpkg)
```
cmake -S . -B build
cmake --build build
```
SDL2 is found via `find_package(SDL2 CONFIG REQUIRED)`; the build links `SDL2::SDL2main` and `SDL2::SDL2`. If you hit an unresolved `main` linker error, make sure `SDL2main` is available (it's what provides the Windows entry point).

### Generating a Visual Studio / Xcode project
```
cmake -S . -B build-xcode -G Xcode          # macOS
cmake -S . -B build -G "Visual Studio 17"   # Windows
```

## Running

```
./build/chip8 <path-to-rom>
```

Currently the ROM path is set in `main.c` (`load_rom(...)`) — edit it to point at your ROM, or wire it to `argv[1]` to pass on the command line.

## Controls

The CHIP-8 hex keypad maps to the left side of a QWERTY keyboard:

```
CHIP-8 keypad        Keyboard
1 2 3 C              1 2 3 4
4 5 6 D      →       Q W E R
7 8 9 E              A S D F
A 0 B F              Z X C V
```

`ESC` (or closing the window) quits.

## Project structure

```
Chip8-c/
├── CMakeLists.txt
├── src/
│   ├── main.c      # SDL window/renderer/texture, event loop, frame timing, keymap
│   ├── chip8.c     # the VM: memory, registers, ROM loading, opcode implementations
│   └── chip8.h     # CHIP8 struct + public function declarations
└── test_roms/      # place .ch8 ROMs here
```

- **`chip8.c`** holds the emulator core: `initialize_chip8`, `load_rom`, `emulate_cpu_cycle` (the fetch/decode/dispatch switch), and one function per opcode.
- **`main.c`** holds everything SDL: it creates the window, runs ~10 CPU cycles per frame, decrements the timers at 60 Hz, converts the `gfx` buffer to pixels, and presents at ~60 fps.

## How it works (short version)

- **Memory:** 4 KB. Font data at `0x050`, ROM loaded at `0x200`, PC starts at `0x200`.
- **Cycle:** each `emulate_cpu_cycle` fetches a 2-byte big-endian opcode, advances PC by 2, then decodes on the top nibble.
- **Display:** a 64×32 `gfx` buffer; `DXYN` XOR-draws sprites and sets `VF` on collision. `draw_flag` tells the render loop when to redraw.
- **Timers:** `delay_timer` and `sound_timer` decrement at 60 Hz (once per frame).
- **Debug logging:** opcode traces are gated behind `#ifdef DEBUG` — build with `-DDEBUG` (or define it) to see per-instruction output.

## Status

**Implemented:**
- Full opcode set: system (`00E0`, `00EE`), flow (`1NNN`, `2NNN`, `BNNN`), skips (`3XNN`, `4XNN`, `5XY0`, `9XY0`), register/constant (`6XNN`, `7XNN`, `ANNN`, `CXNN`), arithmetic/logic (`8XY0`–`8XYE`), draw (`DXYN`), input (`EX9E`, `EXA1`), timers/memory (`FX07`, `FX0A`, `FX15`, `FX18`, `FX1E`, `FX29`, `FX33`, `FX55`, `FX65`)
- SDL2 rendering + keypad input
- 60 Hz timers

**Verified against:** Timendus' `chip8-test-suite` (logo, corax+, flags, quirks, keypad).

**Quirks:** shifts (`8XY6`/`8XYE`) shift `VX` in place; load/store (`FX55`/`FX65`) leave `I` unchanged — the modern / CHIP-48 behavior.

**Possible future work:**
- Read ROM path from `argv[1]`
- Real audio for the sound timer (currently a `BEEP` print stub)
- Configurable CPU speed (instructions per frame)

## Test ROMs

Grab **Timendus' `chip8-test-suite`** (GitHub, Releases tab) for the numbered diagnostic ROMs, and any public-domain CHIP-8 ROM pack for games (Pong, Tetris, Space Invaders, Breakout). Drop them in `test_roms/`.

## Acknowledgements

- Timendus' CHIP-8 test suite for the diagnostic ROMs.
- The NESdev / CHIP-8 community documentation for the opcode and quirk references.
