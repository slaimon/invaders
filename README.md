# Space Invaders Emulator

This is an emulator for the original Space Invaders machine based on the Intel 8080.

I originally wrote it in 2019 but never got it to display anything, only running a couple test programs made for the CP/M operating system. So I'm tidying it up a bit and trying to get it to work.

I tried to write it like a library so as to be able to easily adapt it to other machines based on the same processor. For instance, I'd like to be able to run a simple Altair 8800 emulator.

There is one example so far: it creates a disassembly of the original Space Invaders program. You can run it by doing `make invaders_listing` and the code will be in the file `invaders_listing.txt` thus created.

You can also do `make ./disassembler` to create the disassembler program. It takes two arguments: the input file and the output file names. For instance, `./disassembler INVADERS invaders_listing.txt` is the command executed by the above target.