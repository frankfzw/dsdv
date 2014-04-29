#include "rip.h"
#include "socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

mutex mtx;

//global variable

//store the table from disk temporarily
map<string, struct RouterTab> myTable;

//store adjacent nods map
map<string, struct RouterTab> adjBuf;

//store the route table
map<string, struct RouterTab> routerTable;

//store the map from port number to the name of router
map<int, string> portToName;

//store the host message
struct Host host;

//store the socket fd
int fd;

//store the print time
int printNum;


void* receive(void* ptr)
{
	//cout<<"thread 1\n";
	
	while(1)
	{
		string content;
		int fromPort = socketReceive(fd, content);
		//cout<<"receive from: "<<fromPort<<endl;
		//cout<<"receive content: "<<content<<endl;
		//cout<<"========================\n";
		
		//update route table
		string src = portToName.find(fromPort)->second;
		Host srcRouter;
		srcRouter.name = src;
		map<string, struct RouterTab> srcTab;
		
		//cout<<"========================\n";
		//cout<<"receive from: "<<src<<endl;
		//cout<<"receive content: "<<content<<endl;
		//cout<<"========================\n";
		
		ripReceive(content, srcTab, srcRouter);
		mtx.lock();
		ripUpdate(srcRouter, srcTab, adjBuf, routerTable, host);
		mtx.unlock();
		
		/*
		string sentContent;
		ripSend(sentContent, routerTable);
		//cout<<sentContent;
		if(fromPort != -1)
			socketSend(fd, fromPort, sentContent);
		*/
	}
	
}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("usage: %s <prot_number> <file_name>\n", argv[0]);
		exit(-1);
	}
	
	int port = atoi(argv[1]);
	if(port <= 0)
	{
		printf("error: invalid <port_number>\n");
		exit(-1);
	}
	
	string filename = string(argv[2]);
	
	try
	{
		//init
		
		ripInit(filename, routerTable, portToName, host);
		ripInit(filename, adjBuf, portToName, host);
		cout<<"This is router "<<host.name<<" listening port "<<port<<endl;
	
		fd = socketBind(port);
		int fromPort = -1;
		
		pthread_t child;
		pthread_create(&child, NULL, receive, NULL);
		
		while(1)
		{
			//refresh the table from data
			Host temp;
			myTable.clear();
			mtx.lock();
			ripInit(filename, myTable, portToName, temp);
			ripRefresh(adjBuf, myTable, routerTable, host);
			mtx.unlock();
			
			//print table
			ripPrintTable(routerTable, printNum, host);
			
			//prepare to send
			string sendContent;
			ripSend(sendContent, routerTable, host);
			
			//send router table to adjacent nodes
			map<int, string>::iterator it = portToName.begin();
			for(; it != portToName.end(); it ++)
			{
				socketSend(fd, it->first, sendContent);
			}
			
			sleep(1);
		}
		close(fd);
	}
	catch(int e)
	{
		cout<<"Oops! error code: "<<e<<endl;
	}
    return 0;    
}
