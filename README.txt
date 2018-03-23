To run this project please perform the following steps in order:
1. In the command line type "make ftserver" without quotes.
2. Run the ftserver function by typing "ftserver <port number>" where <port number> is some number between 1024 and 65535
3. Run the ftclient function to get a listing of the directory by typing "python ftclient.py <host> <port number> -l <new port number>"
4. For step 3, <host> is the host server running ftserver, <port number> is the one from step 2, and <new port number> is a new number between 1024 and 65535.
5. In lieu of step 3 you can tun the ftclient function to get a file transfer by typing "python ftclient.py <host> <port number> -l <filename> <new port number>"
6. For step 5, <host> is the host server running ftserver, <port number> is the one from step 2, and <new port number> is a new number between 1024 and 65535 and <filename> is the name of a file.
7. the python should automatically close itself. The ftserver will need you to enter "Ctrl + C" to end.
8. ENJOY!