OBJ = $(patsubst %.c, %.o, $(wildcard *.c))
EXE = files.exe

CFLAGS += -I../../sgfx -I../libc/include
LDFLAGS += -L../../sgfx -lsgfx -L../libc -lc

all: $(EXE)

$(EXE): $(OBJ)
	$(LD) -o $(EXE) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f *.o $(EXE)
