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

./disassembler: $(BIN)/disassembler.o $(BIN)/disassembler8080.o $(BIN)/instruction8080.o $(BIN)/bytestream.o $(BIN)/safe.o
	$(CC) $(CFLAGS) $^ -o $@
$(BIN)/disassembler.o: $(EXAMPLES)/disassembler.c $(HDR)/disassembler8080.h $(HDR)/bytestream.h
	$(CC) $(CFLAGS) -c $(EXAMPLES)/disassembler.c -o $(BIN)/disassembler.o

$(BIN)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BIN)/disassembler8080.o: $(HDR)/disassembler8080.h
$(BIN)/instruction8080.o: $(HDR)/instruction8080.h
$(BIN)/bytestream.o: $(HDR)/bytestream.h
$(BIN)/safe.o: $(HDR)/safe.h

clean:
	rm $(BIN)/*.o
	rm -f ./disassembler
	rm -f ./invaders_listing.txt