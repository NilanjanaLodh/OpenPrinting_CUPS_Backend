FLAGS=$(shell pkg-config --libs --cflags gio-2.0 gio-unix-2.0 glib-2.0)
.PHONY:all gen lib

all: print_backend_cups print_frontend

gen:genback genfront

genfront:
	gdbus-codegen --generate-c-code frontend_interface  --interface-prefix org.openprinting org.openprinting.Frontend.xml

genback:
	gdbus-codegen --generate-c-code backend_interface  --interface-prefix org.openprinting org.openprinting.Backend.xml

%.o: %.c
	gcc -o $@ $^ -c $(FLAGS)

print_backend_cups: print_backend_cups.o backend_interface.o common_helper.o backend_helper.o
	gcc -o $@ $^ $(FLAGS) -lcups

print_frontend: print_frontend.o backend_interface.o frontend_interface.o common_helper.o frontend_helper.o
	gcc -o $@ $^ $(FLAGS)

clean_gen:
	rm -f backend_interface.* frontend_interface.*

clean:
	rm -f *.o print_backend_cups print_frontend
