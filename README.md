# PrintDialog_Backend

Contains implementation of the CUPS and IPP Backends for the Common Printing Dialog Project (as a part of GSoC 2017)
This README just contains information on building and running the project. For more details see [Project Wiki](https://github.com/NilanjanaLodh/PrintDialog_Backend/wiki  "Project Wiki")

Build and installation
----
- make gen   _(Generates the interface code from the interface definition)_
- make       
- sudo make install _(Installs the backends)_
- make release  _(Generates the headers and libraries in the /release folder)_



Running
----
The backend is auto activated on running the frontend, and automatically exits when no frontend is running.
Simply run the frontend:
$ ./print_frontend

The list of printers discovered should start appearing automatically.

type __help__ to get a list of available commands at the backend.


To run multiple frontends simultaneously, supply an extra argument denoting the instance number. For example:
-  ./print_frontend 1

In another terminal: 
-  ./print_frontend 2


**NOTE** : 
- Make sure you exit all the frontends using the __stop__ command only; otherwise the backend doesn't get notified and keeps running in the background, hogging resources.
- Apart from the autoactivating method, you can also run the backend first in one terminal and then run the frontend in another. Doing this will allow you to see the debug statements at the backend.


