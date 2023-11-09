OBJ = $(patsubst %.c, %.o, $(wildcard *.c))
EXE = tasks.exe

CFLAGS += -I../../sgfx
LDFLAGS += -L../../sgfx -lsgfx

all: $(EXE)

$(EXE): $(OBJ)
	$(LD) -o $(EXE) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f *.o $(EXE)
