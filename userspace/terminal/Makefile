SOURCES_C = $(patsubst %.c, %.o, $(wildcard *.c))

OBJ = $(SOURCES_C)
EXE = terminal.exe

all: $(EXE)

$(EXE): $(OBJ)
	$(LD) -o $(EXE) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm -f *.o $(EXE)
