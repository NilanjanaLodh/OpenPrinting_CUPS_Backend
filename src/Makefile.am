bin_PROGRAMS = print_backend_cups
print_backend_cups_SOURCES = print_backend_cups.c backend_helper.c 
print_backend_cups_CPPFLAGS  = $(CPDBackend_CFLAGS)
print_backend_cups_CPPFLAGS += $(GLIB_CFLAGS)
print_backend_cups_CPPFLAGS += $(GIO_CFLAGS)
print_backend_cups_CPPFLAGS += $(GIOUNIX_CFLAGS)

print_backend_cups_LDADD  =  $(CPDBackend_LIBS)
print_backend_cups_LDADD += -lcups -lpthread -lm -lcrypt
print_backend_cups_LDADD += $(GLIB_LIBS)
print_backend_cups_LDADD += $(GIO_LIBS)
print_backend_cups_LDADD += $(GIOUNIX_LIBS)