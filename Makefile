CC = gcc
CFLAGS = -Wall -pedantic -g

BIN = bin
SRC = source
HDR = include
EXAMPLES = examples

.PHONY: clean listing test log

listing: ./disassembler
	rm -f ./listing.txt
	./disassembler ./assets/INVADERS ./listing.txt

test: ./tester
	./tester ./assets/8080PRE.COM
	./tester ./assets/TEST.COM
	./tester ./assets/8080EX1.COM

log: ./logger
	./logger ./assets/8080EX1.COM ./mylog.txt 1000

./invaders: $(BIN)/invaders.o $(BIN)/soundplayer.o $(BIN)/viewer.o $(BIN)/i8080.o $(BIN)/bytestream.o $(BIN)/safe.o
	$(CC) $(CFLAGS) $^ -o $@ `sdl2-config --cflags --libs`

./logger: $(BIN)/logger.o $(BIN)/i8080_cpm.o $(BIN)/i8080_debug.o $(BIN)/i8080_disassembler.o $(BIN)/i8080.o $(BIN)/bytestream.o $(BIN)/safe.o
	$(CC) $(CFLAGS) $^ -o $@

./tester: $(BIN)/tester.o $(BIN)/i8080_cpm.o $(BIN)/i8080.o $(BIN)/bytestream.o $(BIN)/safe.o
	$(CC) $(CFLAGS) $^ -o $@

./disassembler: $(BIN)/disassembler.o $(BIN)/i8080_disassembler.o $(BIN)/bytestream.o $(BIN)/safe.o
	$(CC) $(CFLAGS) $^ -o $@

./step: $(BIN)/step.o $(BIN)/i8080_debug.o $(BIN)/i8080.o $(BIN)/i8080_disassembler.o $(BIN)/bytestream.o $(BIN)/safe.o
	$(CC) $(CFLAGS) $^ -o $@

$(BIN)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BIN)/invaders.o: $(HDR)/i8080.h $(HDR)/safe.h
$(BIN)/soundplayer.o: $(HDR)/soundplayer.h $(HDR)/safe.h
$(BIN)/viewer.o: $(HDR)/viewer.h
$(BIN)/logger.o: $(EXAMPLES)/logger.c $(HDR)/i8080_cpm.h $(HDR)/i8080_debug.h $(HDR)/i8080.h $(HDR)/bytestream.h $(HDR)/safe.h
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
	rm -f ./disassembler ./listing.txt ./step ./tester ./logger ./mylog.txt ./invaders