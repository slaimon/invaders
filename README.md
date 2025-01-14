# Space Invaders Emulator

This is an emulator for the original Space Invaders machine based on the Intel 8080.

I originally wrote it in 2019 but never got it to display anything, only running a couple test programs made for the CP/M operating system. So I'm tidying it up a bit and trying to get it to work.

I tried to write it like a library so as to be able to easily adapt it to other machines based on the same processor. For instance, I'd like to be able to run a simple Altair 8800 emulator.

There are three examples so far:

- `disassembler` is a disassembler program. It takes two arguments: the input filename and the output filename.
   For instance, `./disassembler INVADERS listing.txt` produces a disassembly of the original Space Invaders program. You can also do `make listing` to directly compile the disassembler and create the file `./listing.txt`.

- `step` is a program that takes one argument, the name of a ROM file, and executes it step by step. After each step a crude TUI (terminal user interface) is printed showing the contents of the CPU registers and a disassembly of the instructions that will be executed next. The program quits when the user types either `Q` or `C` (case insensitive) and presses enter.

- `cpm` is a program that takes the name of a ROM file and executes it like a CP/M program, handling text output. It is specifically made to run the tests in `./assets/`. You can run one of these tests by doing `make test`. 