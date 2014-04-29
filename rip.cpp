#include <fstream>
#include <sstream>
#include "rip.h"
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <iostream>


using namespace std;

void ripInit(string filename, map<string, struct RouterTab> &table, map<int, string> &adjTab, struct Host &host)
{
	//clear data
	//table.clear();
	host.seqNum = 0;
	host.vectorNum = 0;
	ifstream ifs;
	ifs.open(filename.c_str(), ifstream::in);

	if(!ifs.good())
	{
		ifs.close();
		throw NOT_FOUND;
	}

	int lineNum = 0;
	string name;
	ifs>>lineNum>>name;
	host.name = name;
	/*RouterTab self;
	self.dst = name;
	self.cost = 0;
	self.hop = name;
	self.seqNum = 0;
	table[name] = self;*/

	while(lineNum--)
	{
		if(ifs.eof())
		{
			ifs.close();
			throw FILE_ERR;
		}

		string dst = "";
		double cost = 0;
		int portNum = 0;
		ifs>>dst>>cost>>portNum;

		RouterTab tab = {};
		tab.dst = dst;
		if(cost < 0)
			cost = MAX;
		tab.cost = cost;
		tab.hop = dst;
		tab.seqNum = 0;
		table[dst] = tab;
		host.vectorNum ++;

		map<int, string>::iterator it;
		//if((it = adjTab.find(portNum)) == adjTab.end())
		adjTab[portNum] = dst;

	}

	ifs.close();
}


void ripSend(string &dst, map<string, struct RouterTab> &table, struct Host &host)
{
	ostringstream oss;
	oss<<host.vectorNum<<endl<<host.seqNum<<endl;
	map<string, struct RouterTab>::iterator it = table.begin();
	//int length = 0;
	for(;it != table.end(); it ++)
	{
		oss<<(it->second).dst<<' '<<(it->second).cost<<' '<<(it->second).hop<<' '<<(it->second).seqNum<<'\n';
		//length ++;
	}
	
	//the head  of the content is the number of router vectors 
	//ostringstream temp;
	//temp<<length<<endl;
	//dst = temp.str();
	dst = oss.str();
}

void ripUpdate(struct Host from, map<string, struct RouterTab> &srcTab, map<string, struct RouterTab> &adjTable, map<string, struct RouterTab> &routeTable, struct Host &host)
{
	//host is in the intermidate status
	if(((from.seqNum % 2) == 0) && ((host.seqNum % 2) == 1))
	{
		//routh changing successed
		
		if((srcTab.find(host.name)->second).seqNum == host.seqNum)
		{
			host.seqNum ++;
			(routeTable.find(from.name)->second).seqNum = from.seqNum;
		}
		
		return;
	}
	
	//from is in the intermidate status
	if(((from.seqNum % 2) == 1) && ((host.seqNum % 2) == 0))
	{
		return;
	}
	
	//from is the exactly one
	if(((from.seqNum % 2) == 1) && ((host.seqNum % 2) == 1))
	{
		host.seqNum ++;
		(routeTable.find(from.name))->second.seqNum = from.seqNum;
		return;
	}
	
	//finish the intermidate status
	if((routeTable.find(from.name)->second).seqNum % 2)
	{
		(routeTable.find(from.name)->second.seqNum) = from.seqNum;
		return;
	}

	//1st: if from is a new one, change the route to directly
	if(from.seqNum > (routeTable.find(from.name)->second).seqNum)
	{
		(routeTable.find(from.name)->second).seqNum = from.seqNum;
		//change all route to adj directly
		map<string, struct RouterTab>::iterator it = adjTable.begin();

		for(; it != adjTable.end(); it ++)
		{
			string name = (it->second).dst;
			double oldCost = (routeTable.find(name)->second).cost;
			double newCost = (it->second).cost;
			(routeTable.find(name)->second).cost = newCost;
			(routeTable.find(name)->second).hop = name;
			map<string, struct RouterTab>::iterator itt = routeTable.begin();
			for(; itt != routeTable.end(); itt ++)
			{
				if(name.compare((itt->second).dst) == 0)
					continue;
				if(name.compare((itt->second).hop) != 0)
					continue;
					
				//cout<<"eeeeeee\t"<<itt->second.cost<<'\t'<<newCost<<'\t'<<oldCost<<endl;	
				
				(itt->second).cost += newCost - oldCost;
				
				
				if((itt->second).cost > MAX)
					(itt->second).cost = MAX;
			}

		}
	}
	else if(from.seqNum == (routeTable.find(from.name)->second).seqNum)
	{
		//adj node didn't finish 1st step
		if((srcTab.find(host.name)->second).seqNum < host.seqNum)
			return;

		map<string, struct RouterTab>::iterator it;
		map<string, struct RouterTab>::iterator it2;
		double fromCost = (routeTable.find(from.name)->second).cost;
		for(it2 = srcTab.begin(); it2 != srcTab.end(); it2 ++)
		{
			string name = (it2->second).dst;
			string hop = (it2->second).hop;
			it = routeTable.find(name);
			
		
			//route back to host
			if(name.compare(host.name) == 0)
				continue;

			//it's a new route
			if(it == routeTable.end())
			{
				RouterTab temp;
				temp.dst = name;
				temp.hop = from.name;
				double newCost = fromCost + (it2->second).cost;
				newCost = (newCost > MAX) ? MAX : newCost;
				temp.cost = newCost;
				
				//cout<<"aaaaaaa\t"<<newCost<<endl;
				
				temp.seqNum = (it2->second).seqNum;
				routeTable[name] = temp;
				host.vectorNum ++;
			}

			//the route is existed
			else
			{
				//ignore the route which pass by it self
				if(hop.compare(host.name) == 0)
					continue;

				//ignore the old route path
				if((it2->second).seqNum < (it->second).seqNum)
					continue;
				
				//froce updating
				else if((it2->second).seqNum > (it->second).seqNum)
				{
					double newCost = fromCost + (it2->second).cost;
					newCost = (newCost > MAX) ? MAX : newCost;
					(it->second).cost = newCost;
					
					//cout<<"bbbbbbb\t"<<newCost<<endl;
					
					(it->second).seqNum = (it2->second).seqNum;
					(it->second).hop = from.name;
				}
				else
				{
					//update route cost and seqNum
					double newCost = fromCost + (it2->second).cost;
					newCost = (newCost > MAX) ? MAX : newCost;
					
					//cout<<"ccccccc\t"<<fromCost<<'\t'<<(it2->second).cost<<endl;
					
					//route don't change, just update the cost
					if(((it->second).hop).compare(from.name) == 0)
					{
						(it->second).cost = newCost;
						//return;
					}
					
					//new path is better
					if(newCost < (it->second).cost)
					{
						(it->second).cost = newCost;
						
						//check if host to from route directly
						string hopHtoF = (routeTable.find(from.name)->second).hop;
						if(hopHtoF.compare(from.name) == 0)
						{
							(it->second).hop = from.name;
						}
						else
							(it->second).hop = hopHtoF;
						//cout<<"here!!!!!!!\nfrom:\t"<<from.name<<endl<<it->second.dst<<'\t'<<it->second.hop<<endl;
						//(it->second).seqNum = srcTab[i].seqNum;
					}
			
				}
			}
		}
	}


}

void ripRefresh(map<string, struct RouterTab> &bufferdAdj, map<string, struct RouterTab> &newAdj, map<string, struct RouterTab> &table, struct Host &host)
{
	string name;
	double newCost;
	double oldCost;

	map<string, struct RouterTab>::iterator it = bufferdAdj.begin();
	for(; it != bufferdAdj.end(); it ++)
	{
		double cost = (it->second).cost;
		name = (it->second).dst;
		newCost = (newAdj.find(name)->second).cost;
		if(abs(cost - newCost) < 0.000001)
			continue;
		else
		{
			//table.clear();
			//table = newAdj;
			//host.seqNum ++;
			break;
		}
	}

	//table didn't change
	if(it == bufferdAdj.end())
		return;
	

	//change all route to adj directly
	for(it = newAdj.begin(); it != newAdj.end(); it ++)
	{
		name = (it->second).dst;
		newCost = (it->second).cost;
		oldCost = (table.find(name)->second).cost;
		(table.find(name)->second).cost = newCost;
		(table.find(name)->second).hop = name;
		map<string, struct RouterTab>::iterator itt = table.begin();
		for(; itt != table.end(); itt ++)
		{
			if(name.compare((itt->second).dst) == 0)
				continue;
			if(name.compare((itt->second).hop) != 0)
				continue;
			(itt->second).cost += newCost - oldCost;
			
			//cout<<"ddddddd\t"<<(itt->second).cost<<endl;
			
			if((itt->second).cost > MAX)
				(itt->second).cost = MAX;
				
				
			
		}

	}
	bufferdAdj = newAdj;
	host.seqNum ++;
}

void ripReceive(string content, map<string, struct RouterTab> &srcTab, struct Host &from)
{
	istringstream iss(content);

	iss>>from.vectorNum>>from.seqNum;
	int length = from.vectorNum;

	while(length --)
	{
		RouterTab temp;
		iss>>temp.dst>>temp.cost>>temp.hop>>temp.seqNum;
		srcTab[temp.dst] = temp;
	}
}

void ripPrintTable(map<string, struct RouterTab> &table, int &printNum, struct Host &host)
{
	printNum ++;
	cout<<"##print-out number\t"<<printNum<<endl;
	map<string, struct RouterTab>::iterator it = table.begin();
	for(; it != table.end(); it ++)
	{
		string dst = (it->first);
		string hop = (it->second).hop;
		int seqNum = (it->second).seqNum;
		double cost = (it->second).cost;
		cout<<"shortest path to node "<<dst
		<<"(seq# "<<seqNum<<"): "
		<<"the next hop is "<<hop
		<<" and the cost is "<<cost<<", "
		<<host.name<<" -> "<<dst<<" : "<<cost<<endl;
	}
}
