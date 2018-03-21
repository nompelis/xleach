CC = gcc
COPTS = -Wall -O0 -fPIC
LIBS = -lX11 -lm 

all:
	$(CC) $(COPTS) -c xroot_stream.c
	$(CC) $(COPTS) -c xwin.c
	$(CC) $(COPTS) main.c xroot_stream.c xwin.o $(LIBS)

clean:
	rm -f *.o a.out

