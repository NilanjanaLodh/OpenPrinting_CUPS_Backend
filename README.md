# PrintDialog_Backend
Contains implementation of the CUPS and IPP Backends for the Common Printing Dialog Project (as a part of GSoC 2017)

Build and installation
----
$ make gen
$ make 
$ sudo chmod u+x install.sh
$ sudo ./install.sh



Running
----
The backend is auto activated on running the frontend, and automatically exits when no frontend is running.
Simply run the frontend:
$ ./print_frontend

The list of printers discovered should start appearing automatically.

type __help__ to get a list of available commands at the backend.


**NOTE** : 
- Make sure you exit the frontend using the __stop__ command only; otherwise the backend doesn't get notified and keeps running in the background, hogging resources.
- Apart from the autoactivating method, you can also run the backend first in one terminal and then run the frontend in another. Doing this will allow you to see the debug statements at the backend.


