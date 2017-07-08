FLAGS=$(shell pkg-config --libs --cflags gio-2.0 gio-unix-2.0 glib-2.0)
.PHONY:all gen make_build_dir

all: create_build_dir print_frontend print_backend_cups

create_build_dir:
	mkdir -p build/

gen:genback genfront

genfront:
	gdbus-codegen --generate-c-code frontend_interface  --interface-prefix org.openprinting interface/org.openprinting.Frontend.xml 
	mv frontend_interface.* src/

genback:
	gdbus-codegen --generate-c-code backend_interface  --interface-prefix org.openprinting interface/org.openprinting.Backend.xml 
	mv backend_interface.* src/

build/%.o: src/%.c
	gcc -o $@ $^ -c $(FLAGS)

print_backend_cups: build/print_backend_cups.o build/backend_interface.o build/common_helper.o build/backend_helper.o
	gcc -o $@ $^ $(FLAGS) -lcups

print_frontend: build/print_frontend.o build/backend_interface.o build/frontend_interface.o build/common_helper.o build/frontend_helper.o
	gcc -o $@ $^ $(FLAGS)

clean_gen:
	rm -f src/backend_interface.* src/frontend_interface.*

clean:
	rm -f print_backend_cups print_frontend build/*

install:
	./install.sh