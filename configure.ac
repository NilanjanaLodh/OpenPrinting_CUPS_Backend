#                                               -*- Autoconf -*- 
# Process this file with autoconf to produce a configure script. 
 
AC_PREREQ([2.69]) 
AC_INIT([COMMON-PRINT-BACKEND-CUPS],[1.0],[nilanjanalodh@gmail.com]) 
AM_INIT_AUTOMAKE([-Wall foreign]) 
AC_CONFIG_SRCDIR([src/print_backend_cups.c]) 

: ${CFLAGS=""}
# Check for a C compiler 
AC_PROG_CC 
 
 
# Checks for backend library 
PKG_CHECK_MODULES([CPDBackend],[CPDBackend]) 
PKG_CHECK_MODULES([GIO],[gio-2.0]) 
PKG_CHECK_MODULES([GIOUNIX],[gio-unix-2.0]) 
PKG_CHECK_MODULES([GLIB],[glib-2.0]) 
 
# Checks for header files. 
AC_CHECK_HEADERS([stdlib.h string.h cups/cups.h]) 
 
# Checks for typedefs, structures, and compiler characteristics. 
AC_TYPE_SIZE_T 
 
# Checks for library functions. 
AC_FUNC_MALLOC 
 
# The Dbus service directory 
AC_DEFINE_UNQUOTED(DBUS_SERVICES_DIR, "/usr/share/dbus-1/services", [Where services dir for DBUS is]) 
 
#The info directory which will be read by the frontend 
AC_DEFINE_UNQUOTED(INFO_DIR, "/usr/share/print-backends", [The directory consulted by the frontends]) 
 
AC_OUTPUT([Makefile src/Makefile data/Makefile])