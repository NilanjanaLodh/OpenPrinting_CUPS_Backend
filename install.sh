# install the binary in /usr/bin/print-backends
echo "installing the cups backend executable in /usr/bin/print-backends .."
mkdir -p /usr/bin/print-backends/
cp print_backend_cups /usr/bin/print-backends/cups

echo "installing interfaces in /usr/share/dbus-1/interfaces .."
cp interface/org.openprinting.Backend.xml /usr/share/dbus-1/interfaces
cp interface/org.openprinting.Frontend.xml /usr/share/dbus-1/interfaces

echo "installing service file org.openprinting.Backend.CUPS.service in /usr/share/dbus-1/services .."
cp aux/org.openprinting.Backend.CUPS.service /usr/share/dbus-1/services

#this is the folder which lets frontends know what backends are there and what are their object paths
echo "installing configuration files in /usr/share/print-backends .."
mkdir -p /usr/share/print-backends
cp aux/org.openprinting.Backend.CUPS /usr/share/print-backends
echo "Done."
