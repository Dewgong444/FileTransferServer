/* Name: Christopher Shelton
 * Date: 3/11/18 
 * Description: This program should make a connection with a client on 
 * one port that is considered the control through which they will exchange
 * messages and requests. On proper command the server will send either the 
 * current directory of its folder or a file to the client depending on which
 * command is used. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int startUp(char* port)
{
	// this whole section is prety much straight from Beej's Guide sections 5.1 to 5.5
	int sockfd;
	struct addrinfo hints, *res;
	//set up the structs
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, port, &hints, &res);
	//socket and check for error (Beej 5.3)
	if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
		return -1;
	//now we bind (Beej 5.3)
	if(bind(sockfd, res->ai_addr, res->ai_addrlen) < 0)
	{
		perror("Unable to bind! \n");
		exit(1);
	}
	//now we listen (Beej 5.5)
	if(listen(sockfd, 1) < 0)
	{
		perror("Unable to listen somehow. \n");
		exit(1);
	}
	//printf("Currently listening...\n");
	//now we need the socket fd back to use in main()
	return sockfd;
}

int getCommand(int sock)
{
	int x = 0;
	int messageSize;
	char commandString[3]; //make sure it's big enough to get "-l" or "-g" and check for errors.
	//using this to help zero my buffers from here on: http://www.daemonology.net/blog/2014-09-04-how-to-zero-a-buffer.html
	memset(commandString, 0, sizeof(commandString));
	//Beej's Guide 6.2 implements a recv
	if(messageSize = recv(sock, commandString, sizeof(commandString)-1, 0) == -1)
	{
		perror("Receive error on command receive.\n");
		exit(1);
	}
	printf("%s \n", commandString);
	//check commandString for "-l" or "-g".
	//Using this for strcmp: https://www.tutorialspoint.com/c_standard_library/c_function_strcmp.htm
	if(strcmp(commandString, "-l") == 0)
	{
		x = 1;
	}
	else if (strcmp(commandString, "-g") == 0)
	{
		x = 2;
	}
	//we assume we have an error unless we get 1 of 2 correct message, -g or -l. A return of 0 == error.
	return x;
}

void sendCommandError(sock)
{
	//Beej's guide
	char* commandError = "ERROR: Please enter a proper command, thank you. Sincerely, -ftserver.c\n";
	send(sock, commandError, strlen(commandError), 0);
	printf("Error sent for invalid command\n");
}

int dirContents(char* path[])
{
	//using this to help with dir and dirent functionality: https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
	int files = 0;
	DIR *d;
	struct dirent *dir;
	d = opendir(".");
    	if (d) 
	{
        	int x = 0;
        	while ((dir = readdir(d)) != NULL) 
		{
            		if (dir->d_type == DT_REG) 
			{
                		path[x] = dir->d_name;
                		x++;
            		}
			//path[x] = dir->d_name;
			//x++;
        	}
        	files = x;
    	}
    	closedir(d);
	return files;
}

void sendData(int sock, char* data[], int files)
{
	int n = 0;
	//printf("Number of files %d \n", files);
	for (n; n < files; n++)
	{
		//Beej's guide
		char* nameOfFile = data[n];
		//int titleSize = sizeof(nameOfFile);
		//send(sock, &titleSize, sizeof(int), 0);
		send(sock, nameOfFile, strlen(nameOfFile), 0); //128 here is the max size of a file name we'll allow for this
		//printf("%s \n", nameOfFile);
		char* newLine = "\n";
		send(sock, newLine, strlen(newLine), 0);
	}
	//send a message to mark the end
	//char * endMessage = "Completed";
	//int messageSize = sizeof(endMessage);
	//send(sock, &messageSize, sizeof(int), 0);
	//send(sock, endMessage, strlen(endMessage), 0);
	//printf("%s \n", endMessage);
	//printf("End message sent!\n");
}

int confirmFileExists(int files, char* data[], char* name)
{
	int i = 0;
	int confirm = 0;
	//loop through filenames for the right file
	for(i; i < files; i++)
	{
		if(strcmp(data[i], name) == 0)
		{
			confirm = 1;
		}
	}	
	return confirm;
}

//using this to help with file commands: https://www.tutorialspoint.com/cprogramming/c_file_io.htm
void sendFile(int sock, char* fileName)
{
	char buffer[2048]; //since python likes recv() in magnitudes of 2 (or so I hear), let's use 2048 as our max file size. 
	FILE *fp;
	memset(buffer, 0, sizeof(buffer));
	fp = fopen(fileName, "r");
	fgets(buffer, 2048, (FILE*)fp); //this should write the file into buffer.
	send(sock, buffer, 2048, 0); //send over the long file string.
}


//All the socket and connection stuff is from Beej's Guide.
int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		perror("ERROR: Must enter as (ftserver <port_number>) only!\n");
		exit(1);
	}
	int isOver = 0;
	//Beej's Guide for all this socket stuff in main()
	char* portNum = argv[1];
	int sockfd = startUp(portNum);
	int connectfd;
	printf("Server open on %s. \n", portNum);
	//printf("startUp() works! \n");
	while(1)
	{
		int dataSock, command;
		int dataConnectfd;
		char dataPort[10];
		//memset the dataport since we need to pass it as a character to the new connection
		memset(dataPort, 0, sizeof(dataPort));
		connectfd = accept(sockfd, NULL, NULL);
		//check for errors
		if(connectfd < 0)
		{
			perror("Accept error.\n");
			exit(1);
		}
		printf("Connection accepted from a client.\n");
		command = getCommand(connectfd);
		if(command == 0)
		{
			sendCommandError(connectfd);
		}
		//printf("Command received\n");
		if(recv(connectfd, dataPort, sizeof(dataPort)-1, 0) == -1)
		{
			perror("ERROR receiving the data port info\n");
			exit(1);
		}
		dataSock = startUp(dataPort);
		printf("Data socket is listening too ...\n");
		while(isOver == 0)
		{
			dataConnectfd = accept(dataSock, NULL, NULL);
			if(dataConnectfd < 0)
			{
				perror("ERROR: Cannot connect on data socket!\n");
				exit(1);
			}
			printf("Data port connected on %s.\n", dataPort);
			if(command == 1) //request for directory
			{
				int dataLength = 0;
				printf("List directory requested on data port.\n");
				int n = 0;
				//Used this to help with path: https://overiq.com/c-programming/101/array-of-pointers-to-strings-in-c/
				char* path[100];
				dataLength = dirContents(path);
				sendData(dataConnectfd, path, dataLength);
				printf("Sending of directory is completed\n");
				//now we sent all the info close the socket.
				close(dataConnectfd);
				isOver = 1; //quit out of this while loop.
			}
			if(command == 2) //request for a file
			{
				int dataLength = 0;
				int n = 0;
				int fileExists = 0;
                                char* path[100];
				char fileName[100];
				memset(fileName, 0, sizeof(fileName));
				//Beej's Guide here again
				if(recv(dataConnectfd, fileName, sizeof(fileName)-1, 0) == -1)
		                {
					perror("Error receiving the file name.\n");
					exit(1);
				}
				printf("File %s requested on data port.\n", fileName);
				//printf("We have the file name\n");
                                dataLength = dirContents(path); //put all the file names in path
				fileExists = confirmFileExists(dataLength, path, fileName);
				//printf("FileExists value is %d \n", fileExists);
				if(fileExists == 0) //file doesn't exist
				{
					printf("Requested file does not exist.\n");
					char* fileError = "File does not exist";
					send(dataConnectfd, fileError, strlen(fileError), 0); 
				}
				else if(fileExists == 1) //file does exist
				{
					printf("Request file exists!\n");
					char* fileGood = "File does exist, now sending...";
					send(dataConnectfd, fileGood, strlen(fileGood), 0); //remember, I like 128. It's a good magnitude of 2, Brent.	
					sendFile(dataConnectfd, fileName);
					printf("File sent!");
				}
				//close the connection.
				close(dataConnectfd);
                                isOver = 1; //quit out of this while loop.
			}
			//isOver = 1;
		}
		close(connectfd);
	}
	return 0;
}
