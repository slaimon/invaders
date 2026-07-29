# Space Invaders Emulator  

An emulator for the original *Space Invaders* arcade machine, built around the Intel 8080
processor.  

I originally started developing it in 2019 and I've been working on it on and off since then.  

This project is fairly modular and could easily be adapted to other 8080-based machines, like an
Altair 8800.

## Features

* full-game emulation with colours and sound
* high-score permanence to file
* both right- and left-handed controls

Additionally, the underlying Intel 8080 emulator features:

* full, cycle-accurate emulation
* automatic tests using original ROM files
* basic TUI debugger and disassembler
* endianness-agnostic code (I think!)

## Play the Game

Find the latest release on the [releases page](https://github.com/slaimon/invaders/releases).
Simply download the right version for your platform, unzip and launch `invaders.exe`.
See `readme.txt` in the release package for more info.

## Building

Make sure you have [SDL3](https://libsdl.org/) installed. Then, run the following commands from
the project's root directory. This will create the release package in `build/release`.

```bash
mkdir build
cmake -B build
cmake --build build --config Release --target release
```

If you're on Windows, you'll need to install SDL3 via vcpkg:
run `vcpkg install sdl3:x86-windows`, then pass your toolchain file to cmake when building,
e.g.:

```bash
cmake --build build --config Release -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Other Programs  

The `examples/` folder contains tools to explore and debug ROMs:

* **`disassembler`** – Converts ROMs into human-readable assembly. Build the utility target
  `listing` to get the full disassembly of `INVADERS.ROM`.
* **`step`** – Runs a ROM step by step, displaying CPU registers and upcoming instructions. Quit
  with `Q` or step ahead with `C`.  
* **`tester`** – Simulates CP/M execution for test programs (use target `test` to run all tests).  
* **`logger`** – Records every CPU step for debugging, useful for comparing against a working
  emulator. The target `log_example` will demonstrate this tool by creating a sample log file.  

All these programs are valid targets for cmake.

## Copyright

I do not own the Space Invaders ROM or the audio files herein included. The copyright belongs
to the respective owners. I will comply with any takedown requests, although they would make me
very very sad.

## References

Each of these resources were invaluable and I learned a lot from them.

* [emulator101.com](https://web.archive.org/web/20241010195903/http://www.emulator101.com/)
* [computerarcheology.com](https://www.computerarcheology.com/)
* [altairclone.com](https://altairclone.com/)
* [begoon's i8080 emulator](https://github.com/begoon/i8080-core/)
