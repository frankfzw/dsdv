#ifndef _RIP_H_
#define _RIP_H_

#include <vector>
#include <string>
#include <map>

using namespace std;

#define NOT_FOUND 0
#define FILE_ERR 1
#define DATA_ERR 2
#define DATA_UNMATCH 3
#define MAX 10000

struct RouterTab{
	string dst;
	string hop;
	int seqNum;
	double cost;
};

struct Host{
	string name;
	int seqNum;
	int vectorNum;
};



/*read the data from disk file and get the name of router*/
void ripInit(string filename, map<string, struct RouterTab> &table, map<int, string> &adjTab, struct Host &host);

/*convert the router table to string*/
void ripSend(string &content, map<string, struct RouterTab> &table, struct Host &host);

/*receive the router table from src router and update own table*/
void ripUpdate(struct Host from, map<string, struct RouterTab> &srcTab, map<string, struct RouterTab> &adjTable, map<string, struct RouterTab> &routeTable, struct Host &host);

/*refresh the route table*/
void ripRefresh(map<string, struct RouterTab> &bufferdAdj, map<string, struct RouterTab> &newAdj, map<string, struct RouterTab> &table, struct Host &host);

/*receive the socket content and turn it to map*/
void ripReceive(string content, map<string, struct RouterTab> &srcTab, struct Host &from);

/*print router table*/
void ripPrintTable(map<string, struct RouterTab> &table, int &printNum, struct Host &host);


#endif
