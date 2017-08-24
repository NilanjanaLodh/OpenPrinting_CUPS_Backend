DIR=$(shell pwd)

GLIB_FLAGS=$(shell pkg-config --cflags --libs gio-2.0 gio-unix-2.0 glib-2.0)
CUPS_FLAGS=$(shell cups-config --cflags --libs)
CPD_BACK_FLAGS=$(shell pkg-config --cflags --libs CPDBackend)

##Installation paths (Where to install the library??)
INSTALL_PATH?=/usr

##for the backends
#Location of the backend executables
##The following paths are fixed(i.e they should not be changed)
BIN_INSTALL_PATH=/usr/bin/print-backends
CONF_PATH=/usr/share/print-backends
SERVICE_PATH=/usr/share/dbus-1/services

all: print_backend_cups 

#compile the cups print backend
print_backend_cups: src/print_backend_cups.c  src/backend_helper.c
	gcc -g -o $@ $^ $(CPD_BACK_FLAGS) $(GLIB_FLAGS) $(CUPS_FLAGS)

##install the cups backend
install:print_backend_cups
	$(info Installing cups backend executable $(BIN_INSTALL_PATH)/cups ..)
	@mkdir -p $(BIN_INSTALL_PATH)	
	@cp print_backend_cups $(BIN_INSTALL_PATH)/cups

	$(info Installing dbus service file in $(SERVICE_PATH) ..)
	@cp aux/org.openprinting.Backend.CUPS.service $(SERVICE_PATH)

	$(info Installing configuration file to be read by frontend in $(CONF_PATH) ..)	
	@mkdir -p $(CONF_PATH)
	@cp aux/org.openprinting.Backend.CUPS $(CONF_PATH)
	$(info Done.)

clean:
	rm -f src/*.o 
	rm -f print_backend_cups print_frontend
	

uninstall:
	$(info uninstalling cups backend..)
	$(info uninstalling cups backend executable from $(BIN_INSTALL_PATH)..)	
	@rm -f $(BIN_INSTALL_PATH)/cups

	$(info uninstalling dbus service file from $(SERVICE_PATH)..)
	@rm -f $(SERVICE_PATH)/org.openprinting.Backend.CUPS.service

	$(info uninstalling configuration file from $(CONF_PATH) ..)	
	@rm -f  $(CONF_PATH)/org.openprinting.Backend.CUPS
	$(info Done.)

