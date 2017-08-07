# PrintDialog_Backend

Contains the project I'm pursuing as a part of GSOC'17.<br/>
This project basically has two major components
- implementation of the CUPS and IPP Backends for the Common Printing Dialog Project(/CUPS_src)
- the frontend and backend CPD interface libraries (/src).<br/>
<br/>
This README just contains information on building and running the project. For more details see [Project Wiki](https://github.com/NilanjanaLodh/PrintDialog_Backend/wiki  "Project Wiki")

### Note on cloning this repo:

First time:<br/>
use `git clone --recursive` to clone this repository. <br/>
For pulling changes, use `git pull --recurse-submodules && git submodule update --recursive`.
Otherwise, the interface/ folder would not get updated.


Build and installation
----
The following command will compile and install libraries, followed by the cups backend.<br/>
`sudo make super`<br/>
That's it! It will set up everything for you.<br/>

However, if you wish to compile and install each component separately, please follow these steps:<br/>
You first need to compile and install the CPD Library; This is one of the dependencies of the backends and the frotend client.<br/>
- `make lib`   _(Compiles the libraries)_
- `sudo make install-lib` _(Installs the libraries)_

Then, you can build the rest of the project:  <br/>
- `make` _(Compiles the cups backend and the sample frontend client)_
- `sudo make install` _(Installs the backends)_

Also, the __(make release)__ command places all the library files into a separate /release folder.


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

Using the frontend and backend libraries
----
To develop a frontend client you need to use the CPDFrontend library.
<br/>It has pkg-config support: `pkg-config --cflags --libs CPDFrontend`
Include `CPDFrontend.h` in your code.
<br/><br/>
Similarly, to develop a backend you need to use the CPDBackend library.
<br/>It has pkg-config support: `pkg-config --cflags --libs CPDBackend`
Include `CPDBackend.h` in your code.

