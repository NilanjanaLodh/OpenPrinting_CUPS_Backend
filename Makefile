FLAGS=$(shell pkg-config --libs --cflags gio-2.0 gio-unix-2.0 glib-2.0)
.PHONY:all gen create_build_dir

all:   print_frontend print_backend_cups


gen:genback genfront

genfront:
	gdbus-codegen --generate-c-code frontend_interface  --interface-prefix org.openprinting interface/org.openprinting.Frontend.xml 
	mv frontend_interface.* src/

genback:
	gdbus-codegen --generate-c-code backend_interface  --interface-prefix org.openprinting interface/org.openprinting.Backend.xml 
	mv backend_interface.* src/

CUPS_src/%.o: CUPS_src/%.c
	gcc -o $@ $^ -c $(FLAGS)

src/%.o: src/%.c
	gcc -o $@ $^ -c $(FLAGS)

print_backend_cups: CUPS_src/print_backend_cups.o src/backend_interface.o src/common_helper.o CUPS_src/backend_helper.o
	gcc -o $@ $^ $(FLAGS) -lcups

print_frontend: src/print_frontend.o src/backend_interface.o src/frontend_interface.o src/common_helper.o src/frontend_helper.o
	gcc -o $@ $^ $(FLAGS)

clean_gen:
	rm -f src/backend_interface.* src/frontend_interface.*

clean:
	rm -f print_backend_cups print_frontend CUPS_src/*.o src/*.o

install:
	./install.sh
