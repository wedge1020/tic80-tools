LIBS = -lm -lgd
CFLAGS = -Wall --std=gnu18 -funsigned-char -Wno-unused-value $(LIBS)
INC = 
CC = gcc
SRC = $(shell /bin/ls -1 *.c 2>/dev/null)
BIN = $(SRC:.c=)
BIN = ticcat ticls ticdump ticify hexcat savehex
BINPATH = .
all: $(SRC) $(BIN) 

debug: CFLAGS += -DDEBUG -g
debug: DEBUG = debug
debug: $(SRC) $(BIN) 

.c:
	$(CC)  $(INC) -o $@ $< $(CFLAGS)

cscript:
	@mkdir -p ../bin
	@for tool in $(BIN); do cat cscript.sh $(TOOL).c >  ../bin/$(TOOL); done
	@chmod 0700 ../bin/*

clean:
	@rm -vf .*.sw[op] *.save* *~ $(BIN) core errors
