.PHONY: all
FLAGS=$(shell pkg-config --libs --cflags gio-2.0 gio-unix-2.0 glib-2.0)

all: server add sub 

gen:
	gdbus-codegen --generate-c-code mydbus --c-namespace MyDBus --interface-prefix com.Nilanjana. com.Nilanjana.xml
	gcc -o $@ $^ -c $(FLAGS)

%.o: %.c
	gcc -o $@ $^ -c $(FLAGS)

server: server.o mydbus.o 
	gcc -o $@ $^ $(FLAGS)
	
add: add.o mydbus.o 
	gcc -o $@ $^ $(FLAGS)

sub: sub.o mydbus.o 
	gcc -o $@ $^ $(FLAGS)

clean:
	rm -f *.o server add sub pingme