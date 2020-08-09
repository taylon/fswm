include config.mk

SOURCES = fswm.c
OBJ = ${SOURCES:.c=.o}

all: fswm run

fswm: ${OBJ}
	@${CC} -o $@ ${CFLAGS} ${SOURCES} ${LIBS} 

clean:
	rm -f fswm

run: 
	@DISPLAY=:1 ./fswm
	
.PHONY: clean
