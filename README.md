# lappd_monThis is the online monitoring code for the 2021 FTBF T1450 Run (LAPPD/mRICH test).
It runs on Martin Purschke's pmonitor software framework and ROOT.

To start, first compile the liblappdmon.so library by doing this:

 # First set up your environment
 export MYINSTALL=<yourdir>/install
 export LD_LIBRARY_PATH=${MYINSTALL}/lib:${LD_LIBRARY_PATH}

 # Then compile and install the library
 mkdir BUILD
 cd BUILD
 ../autogen.sh --prefix=${MYINSTALL}
 make install

Of course you should replace <yourdir> with the directory you'd like as the parent of the install directory.

You should see a new liblappdmon.so library file in your ${MYINSTALL}/lib directory after running
make install.  In general, you only need to compile the library once. However, if you change the source
code, you will have to recompile and reinstall the library file to pick up the changes.

To run the online monitor, go to directory where you will run the online monitoring code from,
and type

  root.exe

At the root prompt (which looks like "root [0]"), execute these lines:

 #include "caen_analysis/caen.h"          // loading the include file
 .L caen_analysis/libcaen.so 

 pfileopen("junk/caen-00000011-0000.evt");    // open up PRDF data file
 pstart(0);                               // starts processing all events in the background

Of course you need to have compiled the caen_analysis code, whi:qch you do by typing "make" in the directory where the code and Makefile are


