# Space Invaders Emulator

This is an emulator for the original Space Invaders machine based on the Intel 8080.

I originally wrote it in 2019 but never got it to display anything, only running a couple test programs made for the CP/M operating system. So I'm tidying it up a bit and trying to get it to work.

I tried to write it like a library so as to be able to easily adapt it to other machines based on the same processor. For instance, I'd like to be able to run a simple Altair 8800 emulator.

The programs in the `examples` folder are meant to illustrate different possible applications of the library. They are:

- `disassembler`, disassembler program. It takes two arguments: the input filename and the output filename.
   For instance, `./disassembler INVADERS listing.txt` produces a disassembly of the original Space Invaders program. You can also do `make listing` to directly compile the disassembler and create the file `./listing.txt`.

- `step`, a program that takes one argument, the name of a ROM file, and executes it step by step. After each step a crude TUI (terminal user interface) is displayed showing the contents of the CPU registers and a disassembly of the instructions that will be executed next. The program quits when the user types either `Q` or `C` (case insensitive) and presses enter.

- `tester`, a program that takes the name of a ROM file and executes it like a CP/M program, handling text output. It is specifically made to run the tests in `./assets/`. You can run all of these tests in sequence by doing `make test`. 

- `logger`, a program to print out a log of all the steps performed by the `tester` program, and the CPU state at each step. It takes three arguments: the input ROM file, the output log file and the maximum number of iterations to perform before halting. This program was especially useful in the early debugging phase of this emulator: if you have another program that does the same thing, but for a working emulator, then all you need to do is compare the two files they produce to find what your implementation is doing wrong. You can run a sample log by doing `make log`. Be careful playing with the `MAX_ITERATIONS` argument: execution logs tend to become very large, very quickly (computers are FAST!)