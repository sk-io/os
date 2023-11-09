SOURCES_C = $(patsubst %.c, %.o, $(wildcard *.c))

OBJ = $(SOURCES_C)
EXE = test.exe

all: $(EXE)

$(EXE): $(OBJ)
	$(LD) -o $(EXE) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f *.o $(EXE)
