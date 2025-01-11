CC = gcc
CFLAGS = -Wall -pedantic -g

BIN = bin
SRC = source
HDR = include
EXAMPLES = examples

.PHONY: clean invaders_listing

invaders_listing: ./disassembler
	rm -f ./invaders_listing.txt
	./disassembler ./INVADERS ./invaders_listing.txt

./disassembler: $(BIN)/disassembler.o $(BIN)/i8080_disassembler.o $(BIN)/i8080_instruction.o $(BIN)/bytestream.o $(BIN)/safe.o
	$(CC) $(CFLAGS) $^ -o $@

$(BIN)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BIN)/disassembler.o: $(EXAMPLES)/disassembler.c $(HDR)/i8080_disassembler.h $(HDR)/bytestream.h
	$(CC) $(CFLAGS) -c $(EXAMPLES)/disassembler.c -o $(BIN)/disassembler.o
$(BIN)/i8080_debug.o: $(HDR)/i8080_disassembler.h $(HDR)/i8080_debug.h
$(BIN)/i8080_disassembler.o: $(HDR)/i8080_disassembler.h
$(BIN)/i8080_instruction.o: $(HDR)/i8080_instruction.h
$(BIN)/bytestream.o: $(HDR)/bytestream.h
$(BIN)/safe.o: $(HDR)/safe.h

clean:
	rm $(BIN)/*.o
	rm -f ./disassembler
	rm -f ./invaders_listing.txt