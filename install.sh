# install the binary in /usr/bin/print-backends
echo "installing the cups backend executable in /usr/bin/print-backends .."
mkdir -p /usr/bin/print-backends/
cp print_backend_cups_new /usr/bin/print-backends/cups

echo "installing service file org.openprinting.Backend.CUPS.service in /usr/share/dbus-1/services .."
cp org.openprinting.Backend.CUPS.service /usr/share/dbus-1/services

#this is the folder which lets frontends know what backends are there and what are their object paths
echo "installing configuration files in /etc/print-backends"
mkdir -p /etc/print-backends
cp org.openprinting.Backend.CUPS /etc/print-backends
echo "Done."