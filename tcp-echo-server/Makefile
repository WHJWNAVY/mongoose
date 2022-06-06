PROG ?= echo-server
CFLAGS ?= -DMG_IO_SIZE=8192 -DMG_ENABLE_LINES

all: $(PROG)
	$(DEBUGGER) ./$(PROG) $(ARGS)

$(PROG): main.c
	$(CC) ../mongoose/mongoose.c -I../mongoose -W -Wall $(CFLAGS) -o $(PROG) main.c

clean:
	rm -rf $(PROG) *.o *.dSYM *.gcov *.gcno *.gcda *.obj *.exe *.ilk *.pdb
