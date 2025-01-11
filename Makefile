CC = gcc
CFLAGS = -Wall -pedantic -g

BIN = bin
SRC = source
HDR = include
EXAMPLES = examples

.PHONY: clean listing

listing: ./disassembler
	rm -f ./listing.txt
	./disassembler ./INVADERS ./listing.txt

./disassembler: $(BIN)/disassembler.o $(BIN)/i8080_disassembler.o $(BIN)/i8080_instruction.o $(BIN)/bytestream.o $(BIN)/safe.o
	$(CC) $(CFLAGS) $^ -o $@

./step: $(BIN)/step.o $(BIN)/i8080_debug.o $(BIN)/i8080.o $(BIN)/i8080_disassembler.o $(BIN)/i8080_instruction.o $(BIN)/bytestream.o $(BIN)/safe.o
	$(CC) $(CFLAGS) $^ -o $@

$(BIN)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BIN)/step.o: $(EXAMPLES)/step.c $(HDR)/i8080_debug.h $(HDR)/safe.h $(HDR)/bytestream.h
	$(CC) $(CFLAGS) -c $(EXAMPLES)/step.c -o $(BIN)/step.o
$(BIN)/disassembler.o: $(EXAMPLES)/disassembler.c $(HDR)/i8080_disassembler.h $(HDR)/bytestream.h
	$(CC) $(CFLAGS) -c $(EXAMPLES)/disassembler.c -o $(BIN)/disassembler.o
$(BIN)/i8080_debug.o: $(HDR)/i8080_disassembler.h $(HDR)/i8080_debug.h
$(BIN)/i8080.o: $(HDR)/i8080.h
$(BIN)/i8080_disassembler.o: $(HDR)/i8080_disassembler.h
$(BIN)/i8080_instruction.o: $(HDR)/i8080_instruction.h
$(BIN)/bytestream.o: $(HDR)/bytestream.h
$(BIN)/safe.o: $(HDR)/safe.h

clean:
	rm $(BIN)/*.o
	rm -f ./disassembler
	rm -f ./listing.txt