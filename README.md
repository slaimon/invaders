# Space Invaders Emulator  

An emulator for the original *Space Invaders* arcade machine, built around the Intel 8080 processor.  

I originally started developing it in 2019 and I've been working on it on and off since then.  

This project is fairly modular and could easily be adapted to other 8080-based machines, like an Altair 8800.

## Playing the Game

Find the latest release on the [releases page](https://github.com/slaimon/invaders/releases).
Simply download the right version for your platform, unzip and launch `invaders.exe`.

* Press `C` to insert coins, then press `1` or `2` to start a one- or two-player game.
* Use the arrow keys or `A-W-D` to play. Press `UP` or `W` to shoot.
* Press `Del` to tilt the game. **Warning:** instant game over!

Your high-score is saved when you quit the game, so you can try to beat it when you come back. You
can also try to edit the savefile (`score.bin`) but watch out! Things can get weird ;)

## Building

I successfully compiled the game on both Linux (using gcc) and Windows (using mingw-gcc).

Make sure you have SDL3 installed (see the following section). Then, run the following commands
from the project's root directory:

```bash
mkdir build
cmake -B build
cmake --build build --config Release --target release
```

This will create the release package in `build/release`.

## Dependencies

The project relies on [SDL3](https://wiki.libsdl.org/SDL3/FrontPage).

At the time of writing this, there are still no pre-built binaries for **Linux**, so you'll need to
[build it yourself](https://github.com/libsdl-org/SDL/blob/main/INSTALL.md). Do check though, as
hopefully this has changed in the meantime.

On **Windows**, you can use vcpkg: run `vcpkg install sdl3:x86-windows`, then pass your toolchain
file to cmake when building, e.g.:

```bash
cmake --build build --config Release --target release -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
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
