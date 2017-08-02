DIR=$(shell pwd)
#these are the glib headers and libraries required by all the files
GLIB_FLAGS=$(shell pkg-config --cflags --libs gio-2.0 gio-unix-2.0 glib-2.0)

##Installation paths (Where to install the library??)
INSTALL_PATH?=/usr
##Libraries
LIB_INSTALL_PATH=$(INSTALL_PATH)/lib
HEADER_INSTALL_PATH=$(INSTALL_PATH)/include/cpd-interface-headers
PKGCONFIG_PATH=$(LIB_INSTALL_PATH)/pkgconfig

##for the backends
#Location of the backend executables
##The following paths are fixed(i.e they should not be changed)
BIN_INSTALL_PATH=/usr/bin/print-backends
CONF_PATH=/usr/share/print-backends
SERVICE_PATH=/usr/share/dbus-1/services

##The compiler and linker flags for making backends
CPD_BACK_FLAGS=$(shell pkg-config --cflags --libs CPDBackend)

##The compiler and linker flags for making frontend clients
CPD_FRONT_FLAGS=$(shell pkg-config --cflags --libs CPDFrontend)

.PHONY : all  \
clean  clean_gen lib install-lib \
install-cups-backend release install uninstall

all: print_backend_cups print_frontend
install: install-cups-backend 

uninstall: uninstall-lib uninstall-cups-backend

#autogenerate the interface code from xml definition 
gen: src/backend_interface.c src/frontend_interface.c

src/frontend_interface.c:interface/org.openprinting.Frontend.xml
	gdbus-codegen --generate-c-code frontend_interface --interface-prefix org.openprinting interface/org.openprinting.Frontend.xml 
	mv frontend_interface.* src/

src/backend_interface.c:interface/org.openprinting.Backend.xml
	gdbus-codegen --generate-c-code backend_interface --interface-prefix org.openprinting interface/org.openprinting.Backend.xml 
	mv backend_interface.* src/

clean-gen:
	rm -f src/*_interface.*

src/%.o: src/%.c
	gcc -fPIC -c -o $@ $^ -I$(DIR)/src $(GLIB_FLAGS) 

#compile the libraries
lib: src/libCPDBackend.so src/libCPDFrontend.so

src/libCPDBackend.so: src/backend_interface.o src/frontend_interface.o src/common_helper.o
	gcc -shared -o $@ $^ $(GLIB_FLAGS)

src/libCPDFrontend.so: src/backend_interface.o src/frontend_interface.o src/common_helper.o src/frontend_helper.o 
	gcc -shared -o $@ $^ $(GLIB_FLAGS)

#install the compiled libraries and their headers
install-lib: src/libCPDBackend.so src/libCPDFrontend.so
	mkdir -p $(LIB_INSTALL_PATH)
	cp src/*.so $(LIB_INSTALL_PATH)
	mkdir -p $(HEADER_INSTALL_PATH)
	cp src/*.h $(HEADER_INSTALL_PATH)
	mkdir -p $(PKGCONFIG_PATH)
	cp src/*.pc $(PKGCONFIG_PATH)

#compile the cups print backend
CUPS_FLAGS=$(shell cups-config --cflags --libs)
print_backend_cups: CUPS_src/print_backend_cups.c  CUPS_src/backend_helper.c
	gcc -g -o $@ $^ $(CPD_BACK_FLAGS) $(GLIB_FLAGS) $(CUPS_FLAGS)

##install the cups backend
install-cups-backend:print_backend_cups
	$(info Installing cups backend executable $(BIN_INSTALL_PATH)/cups ..)
	@mkdir -p $(BIN_INSTALL_PATH)	
	@cp print_backend_cups $(BIN_INSTALL_PATH)/cups

	$(info Installing dbus service file in $(SERVICE_PATH) ..)
	@cp aux/org.openprinting.Backend.CUPS.service $(SERVICE_PATH)

	$(info Installing configuration file to be read by frontend in $(CONF_PATH) ..)	
	@mkdir -p $(CONF_PATH)
	@cp aux/org.openprinting.Backend.CUPS $(CONF_PATH)
	$(info Done.)

##compile the sample frontend
print_frontend: SampleFrontend/print_frontend.c 
	gcc -o $@ $^ $(CPD_FRONT_FLAGS) $(GLIB_FLAGS)

clean:clean-gen
	rm -f src/*.so
	rm -f src/*.o 
	rm -f print_backend_cups print_frontend

uninstall-lib:
	rm -f -r $(HEADER_INSTALL_PATH)
	rm -f $(LIB_INSTALL_PATH)/libCPDBackend.so
	rm -f $(LIB_INSTALL_PATH)/libCPDFrontend.so
	

uninstall-cups-backend:
	$(info uninstalling cups backend..)
	$(info uninstalling cups backend executable from $(BIN_INSTALL_PATH)..)	
	@rm -f $(BIN_INSTALL_PATH)/cups

	$(info uninstalling dbus service file from $(SERVICE_PATH)..)
	@rm -f $(SERVICE_PATH)/org.openprinting.Backend.CUPS.service

	$(info uninstalling configuration file from $(CONF_PATH) ..)	
	@rm -f  $(CONF_PATH)/org.openprinting.Backend.CUPS
	$(info Done.)

release:
	@mkdir -p release/headers
	@mkdir -p release/libs
	cp src/*.h release/headers
	cp src/*.so release/libs

##The following target has been defined just to be useful while developing.
##So that I dont need to install the libraries and the backend separately again and again
##this will be removed later
super: lib install-lib clean all install
