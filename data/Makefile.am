# Dbus service file
servicedir = /usr/share/dbus-1/services
service_in_files = org.openprinting.Backend.CUPS.service.in
service_DATA = $(service_in_files:.service.in=.service)

$(service_DATA): $(service_in_files) Makefile
	@sed -e "s|\@bindir\@|$(bindir)|" $<> $@

info_backenddir = /usr/share/print-backends
info_backend_DATA = org.openprinting.Backend.CUPS

clean-local:
	rm -rf $(service_DATA)