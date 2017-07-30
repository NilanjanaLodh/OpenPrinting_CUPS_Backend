DIR := ${CURDIR}
INCLUDE=-I$(DIR)/src 
INCLUDE+=$(shell pkg-config --cflags gio-2.0 gio-unix-2.0 glib-2.0)

LIB=$(shell pkg-config --libs gio-2.0 gio-unix-2.0 glib-2.0)
LIB+=-L$(DIR)/src/

LIBDIR_FLAG=-Wl,-rpath=$(DIR)/src


.PHONY:all gen release libs

all: lib print_frontend print_backend_cups

gen:genback genfront

genfront: interface/org.openprinting.Frontend.xml
	gdbus-codegen --generate-c-code frontend_interface  --interface-prefix org.openprinting interface/org.openprinting.Frontend.xml 
	mv frontend_interface.* src/

genback: interface/org.openprinting.Backend.xml
	gdbus-codegen --generate-c-code backend_interface  --interface-prefix org.openprinting interface/org.openprinting.Backend.xml 
	mv backend_interface.* src/

%.o: %.c
	gcc -fPIC $(INCLUDE) -o $@ $^ -c 

lib: src/libCPDBackend.so src/libCPDFrontend.so


src/libCPDBackend.so: src/backend_interface.o src/frontend_interface.o src/common_helper.o
	gcc -shared -o $@ $^ $(LIB)

src/libCPDFrontend.so: src/backend_interface.o src/frontend_interface.o src/common_helper.o src/frontend_helper.o 
	gcc -shared -o $@ $^ $(LIB)

print_backend_cups: CUPS_src/print_backend_cups.c  CUPS_src/backend_helper.c
	gcc $(INCLUDE) $(LIBDIR_FLAG) -o $@ $^ -lCPDBackend $(LIB) -lcups 

print_frontend: SampleFrontend/print_frontend.c
	gcc $(INCLUDE) $(LIBDIR_FLAG) -o $@ $^ -lCPDFrontend $(LIB)

clean_gen:
	rm -f src/backend_interface.* src/frontend_interface.*

clean:
	rm -f print_backend_cups print_frontend CUPS_src/*.o src/*.o src/*.so SampleFrontend/*.o
	
clean_release:
	rm -f release/libs/* 
	rm -f release/headers/*

install:
	./install.sh

release: src/libCPDBackend.so src/libCPDFrontend.so src/*.h
	mkdir -p release/libs
	mkdir -p release/headers
	cp src/*.so release/libs
	cp src/*.h release/headers
