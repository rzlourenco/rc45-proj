# Important to Read

The storage server needs to know the central server's address so it
can be "authenticated". Please edit the CS_ADDR constant as needed in
common.py. The format is (IP, PORT).

The central server needs a list of storage servers, in a file called serverlist.
The syntax is hostname:port, one per line. The file needs to be one folder above,
so it does not polute the file list, since the central server touches the file
names that are kept. This can be changed in central.py, in the read_storage_servers
function.

*** DON'T RUN EVERYTHING IN THE SAME WORKING DIRECTORY ***
The testing folder structure was:
  testarea/
      central/
      storage0/
      storage1/
      storage2/
      storage3/
      user/
      serverlist

Each program should be run in it's respective working directory.

The overall architecture could be vastly improved, but this was mostly hacked
together :) 

