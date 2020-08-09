X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

INCLUDES = -I${X11INC}
LIBS = -L${X11LIB} -lX11

CFLAGS = -std=c99 -pedantic -Wall -Os ${INCLUDES} 

CC = cc
