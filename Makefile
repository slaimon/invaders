CC = gcc
CFLAGS = -Wall -pedantic -g

BIN = bin
SRC = source
HDR = include
EXAMPLES = examples

.PHONY: clean listing test

listing: ./disassembler
	rm -f ./listing.txt
	./disassembler ./assets/INVADERS ./listing.txt

test: ./tester
	./tester ./assets/TEST.COM
	./tester ./assets/8080EX1.COM

print_test: ./printexec
	./printexec ./assets/8080EX1.COM ./out.txt

./printexec: $(BIN)/printexec.o $(BIN)/i8080_cpm.o $(BIN)/i8080_debug.o $(BIN)/i8080_disassembler.o $(BIN)/i8080.o $(BIN)/bytestream.o $(BIN)/safe.o
	$(CC) $(CFLAGS) $^ -o $@

./tester: $(BIN)/tester.o $(BIN)/i8080_cpm.o $(BIN)/i8080.o $(BIN)/bytestream.o $(BIN)/safe.o
	$(CC) $(CFLAGS) $^ -o $@

./disassembler: $(BIN)/disassembler.o $(BIN)/i8080_disassembler.o $(BIN)/bytestream.o $(BIN)/safe.o
	$(CC) $(CFLAGS) $^ -o $@

./step: $(BIN)/step.o $(BIN)/i8080_debug.o $(BIN)/i8080.o $(BIN)/i8080_disassembler.o $(BIN)/bytestream.o $(BIN)/safe.o
	$(CC) $(CFLAGS) $^ -o $@

$(BIN)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BIN)/printexec.o: $(EXAMPLES)/printexec.c $(HDR)/i8080_cpm.h $(HDR)/i8080_debug.h $(HDR)/i8080.h $(HDR)/bytestream.h $(HDR)/safe.h
	$(CC) $(CFLAGS) -c $< -o $@
$(BIN)/tester.o: $(EXAMPLES)/tester.c $(HDR)/i8080_cpm.h $(HDR)/i8080.h $(HDR)/bytestream.h $(HDR)/safe.h
	$(CC) $(CFLAGS) -c $< -o $@
$(BIN)/step.o: $(EXAMPLES)/step.c $(HDR)/i8080_debug.h $(HDR)/safe.h $(HDR)/bytestream.h
	$(CC) $(CFLAGS) -c $< -o $@
$(BIN)/disassembler.o: $(EXAMPLES)/disassembler.c $(HDR)/i8080_disassembler.h $(HDR)/bytestream.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN)/i8080_cpm.o: $(HDR)/i8080_cpm.h $(HDR)/i8080.h
$(BIN)/i8080_debug.o: $(HDR)/i8080_disassembler.h $(HDR)/i8080_debug.h
$(BIN)/i8080_disassembler.o: $(HDR)/i8080_disassembler.h
$(BIN)/i8080.o: $(HDR)/i8080.h
$(BIN)/bytestream.o: $(HDR)/bytestream.h
$(BIN)/safe.o: $(HDR)/safe.h

clean:
	rm -f $(BIN)/*.o
	rm -f ./disassembler ./listing.txt ./step ./tester ./printexec ./out.txt