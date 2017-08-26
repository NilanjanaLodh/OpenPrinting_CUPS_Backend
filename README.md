# CUPS, IPP Backend for Common Print Dialog

This repository contains one of the two components of my Google Summer of Code'17 project with The Linux Foundation, i.e the CUPS Backend. The other component can be found [here](https://github.com/NilanjanaLodh/OpenPrinting_CPD_Libraries).

## Background

### The Problem

Printing out of desktop applications is managed by many very different dialogs, mostly depending on which GUI toolkit is used for an application. Some applications like even have their own dialogs, which expose different kind of printing options. This is confuses users a lot, having them to do the printing operation in many different ways. In addition, many dialogs are missing important features.

### The solution

The [Common Printing Dialog](https://wiki.ubuntu.com/CommonPrintingDialog) project aims to solve these problems provide a uniform printing experience on Linux Desktop Environments.

### My contributions

I specifically have contributed to the project in the following two ways :

- Developed backend frontend  libraries which provides allows the frontend and backend to communicate to each other over the D-Bus. Available [here](https://github.com/NilanjanaLodh/OpenPrinting_CPD_Libraries).
- Developed the CUPS Backend, i,e. the repository you are looking at now.

## Dependencies

- The [Common Printing Library](https://github.com/NilanjanaLodh/OpenPrinting_CPD_Libraries) develped by me as a part of   GSOC'17. Follow the README at the given link for installation procedure of the library.
- [CUPS](https://github.com/apple/cups/releases) : Version >= 2.2 
 Install bleeding edge release from [here](https://github.com/apple/cups/releases).
 OR
`sudo apt install cups libcups2-dev`

- GLIB 2.0 :
`sudo apt install libglib2.0-dev`

## Build and installation

    $ ./autogen.sh
    $ ./configure
    $ make
    $ sudo make install


## Running

The backend is auto-activated when a frontend runs; So no need to run it explicitly.
However, if you wish to see the debug statements at the backend, you can run  `print_backend_cups`. 

For a sample command line frontend client , look at [my other repository](https://github.com/NilanjanaLodh/OpenPrinting_CPD_Libraries).