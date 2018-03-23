#Name: Chris Shelon
#Date: 3/11/18
#Description: This is the client companion to ftserver.c. 
#It should make a connection with the server and then send commands
#On sending commands for a directory list or file transfer, it should receive those through a data connection.
#It should also display to the user the received messages.

import sys
import socket
from os import path
from time import sleep

#I use this link to help with all my socket stuff: https://docs.python.org/2/library/socket.html
#make these global variables to easily use around the program
hostName = ""
portNum = 0
command = ""
fileName = ""
dataPortNum = 0

def verifyInput():
	 #now we check to make sure the format is correct, becuase if it's not of length 5 or 6 something is wrong.
	if len(sys.argv) != 5 and len(sys.argv) != 6:
		print "ERROR: Incorrect format. Please enter as (python ftclient.py <hostname> <port_number> <command> <filename> <data_port_number>"
        	sys.exit(1)
	global hostName 
	hostName = sys.argv[1]
	global portNum 
	portNum = int(sys.argv[2])
	global command 
	command = sys.argv[3]
	global fileName
	global dataPortNum
	#now, depending on command it'll have filename or not, so put a condition for that once we prep those variables
	#fill these in using the conditional(s) below
	if len(sys.argv) is 6:
		fileName = sys.argv[4]
		dataPortNum = int(sys.argv[5])
	elif len(sys.argv) is 5:
		dataPortNum = int(sys.argv[4])
	#check for proper port numbers
	if portNum < 1024 or portNum > 65535 or dataPortNum < 1024 or dataPortNum > 65535:
		print "ERROR: Your port numbers (both of them!) MUST be between 1024 and 65535. It's not difficult!"
		sys.exit(1)
	#print "So far so good!"
		
def startUp(port, host):
	#used this link to help here: https://www.geeksforgeeks.org/simple-chat-room-using-python/
	firstSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	firstSocket.connect((host, port))
	return firstSocket

def sendCommand(sock):
	global command
	print command
	#we need to make sure we receive the error message from the server if we have the wrong command
	if command != "-l" and command != "-g":
		#print "ERROR: Please enter the command portion as -l or -g"
		sock.send(command);
		#using this to deal with socket stuff and python: https://docs.python.org/3/library/socket.html
		errorResponse = sock.recv(1024)
		print errorResponse
		sys.exit(1)
	else:
		sock.send(command)
		#print "Sent command."

def sendDataPort(sock):
	global dataPortNum
	sock.send(str(dataPortNum))

def receiveDirectory(server):
	#receive the directory specifically
	#so let's get the first file, remember we're expecting a size of 128 here from the server:
	dirName = ""
	dirName = server.recv(2048)
	print dirName 
	#well this stuff did not work and was very frustrating.
	#start up a while loop that ends when we get the completed message
	#while dirName != "Completed":
	#	print dirName
	#	dirName = server.recv(256)
	print "End of directory."

def receiveFile(server):
	sleep(1)
	server.send(fileName)
	#using this to help with python files:http://www.pythonforbeginners.com/files/reading-and-writing-files-in-python
	#receive the confirmation or error message
	recvMessage = server.recv(128)
	print recvMessage
	if recvMessage == "File does not exist":
		print "Server responded that file does not exist!"
		sys.exit(1)
	else:
		newFile = open(fileName, "w")
		fileMessage = server.recv(2048) #read that python likes magnitudes of 2 here, don't know exactly why
		newFile.write(fileMessage)
		print "File received!"

def main():
        quitMessage = '\quit'
	verifyInput()
        server = startUp(portNum, hostName)
	#print command
	sendCommand(server)
	sendDataPort(server)
	#we need to give the other program time to set up the other socket
	sleep(1)
	newServer = startUp(dataPortNum, hostName)
	if command == "-l":
		receiveDirectory(newServer)
	if command == "-g":
		receiveFile(newServer)
	#print "we did it!"
	
#added this just in case, from: https://stackoverflow.com/questions/4041238/why-use-def-main
if __name__ == "__main__":
        main()

