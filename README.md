# Space Invaders Emulator

This is an emulator for the original Space Invaders machine based on the Intel 8080.

I originally wrote it in 2019 but never got it to display anything, only running a couple test programs made for the CP/M operating system. So I'm tidying it up a bit and trying to get it to work.

I tried to write it like a library so as to be able to easily adapt it to other machines based on the same processor. For instance, I'd like to be able to run a simple Altair 8800 emulator.

There are two example so far:

- `make disassembler` creates a disassembler program. It takes two arguments: the input filename and the output filename.
   For instance, `./disassembler INVADERS listing.txt` produces a disassembly of the original Space Invaders program. You can also do `make listing` to directly compile the disassembler and create the file `./listing.txt`.

- `make step` creates a program that takes one argument, the name of a file containing Intel 8080 machine code, and executes it step by step. After each step a crude TUI (terminal user interface) is printed showing the contents of the CPU registers and a disassembly of the instructions that will be executed next. The program quits when the user types either `Q` or `C` (case insensitive) and presses enter.