DIR := ${CURDIR}
FLAGS=$(shell pkg-config --libs --cflags gio-2.0 gio-unix-2.0 glib-2.0)
FLAGS+=-I$(DIR)/src
FLAGS+=-L$(DIR)/src


.PHONY:all gen release

all:   print_frontend print_backend_cups release


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

SampleFrontend/%.o: SampleFrontend/%.c
	gcc -o $@ $^ -c $(FLAGS)

src/libCPDcore.a: src/backend_interface.o src/frontend_interface.o src/common_helper.o
	ar rcs -o src/libCPDcore.a $^

src/libCPD.a: src/backend_interface.o src/frontend_interface.o src/common_helper.o src/frontend_helper.o 
	ar rcs -o src/libCPD.a $^

print_backend_cups: CUPS_src/print_backend_cups.o  CUPS_src/backend_helper.o src/libCPDcore.a
	gcc -o $@ $^ $(FLAGS) -lcups -lCPDcore

print_frontend: SampleFrontend/print_frontend.o src/libCPD.a
	gcc -o $@ $^ $(FLAGS) -lCPD

clean_gen:
	rm -f src/backend_interface.* src/frontend_interface.*

clean:
	rm -f print_backend_cups print_frontend CUPS_src/*.o src/*.o src/*.a SampleFrontend/*.o

install:
	./install.sh

release: src/libCPD.a src/libCPDcore.a 
	cp src/*.a release/libs
	cp src/*.h release/headers