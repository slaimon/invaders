# Space Invaders Emulator  

An emulator for the original *Space Invaders* arcade machine, built around the Intel 8080 processor.  

I originally started developing it in 2019 and I've been working on it on and off since then.  

This project is fairly modular and could easily be adapted to other 8080-based machines, like an Altair 8800.

### Playing the Game

To compile and run the game:
```
make invaders
./invaders
```
Press `C` to insert coins, `1` or `2` to start a one- or two-player game.
* Player one uses the arrow keys.
* Player two uses `A`, `W`, `D` (untested).

### Other Programs  

The `examples/` folder contains tools to explore and debug ROMs:

- **`disassembler`** – Converts ROMs into human-readable assembly (e.g., `make listing`).  
- **`step`** – Runs a ROM step by step, displaying CPU registers and upcoming instructions. Quit with `Q` or `C`.  
- **`tester`** – Simulates CP/M execution for test programs (`make test` to run all).  
- **`logger`** – Records every CPU step for debugging, useful for comparing against a working emulator (`make log`).  

All these programs are valid targets for `make`.
