

文件构成：
	本次lab主要由5个文件构成
	socket.h & socket.cpp用来提供udp socket方面的接口和实现，具体内容可以参考socket.h
	
	rip.h & rip.cpp用来提供路由算法的接口和实现，主要包含两个struct和6个函数接口
	
		struct RouterTab:用来保存路由表中每一行的数据，其中seqNum表示重点的seqNum
		struct Host:用来表示主机，包括主机的名称，seqNum和路由表的行数
		
		void ripInit(string filename, map<string, struct RouterTab> &table, map<int, string> &adjTab, struct Host &host)
			用来初始化路由器，从filename中读入表项并将其保存到table中，将端口号和名称的映射保存在adjTab中，将host信息保存到host中
		void ripSend(string &content, map<string, struct RouterTab> &table, struct Host &host)
			用来住准备发送内容，将主机信息和路由表转换成string保存到content中
		void ripUpdate(struct Host from, map<string, struct RouterTab> &srcTab, map<string, struct RouterTab> &adjTable, map<string, struct RouterTab> &routeTable, struct Host &host)
			当收到邻节点from的路由表srcTab是，根据主机保存的信息更新
		void ripRefresh(map<string, struct RouterTab> &bufferdAdj, map<string, struct RouterTab> &newAdj, map<string, struct RouterTab> &table, struct Host &host)
			根据重新从磁盘读入的newAdj和之前保存的bufferAdj来更新host的信息
		void ripReceive(string content, map<string, struct RouterTab> &srcTab, struct Host &from);
			将收到邻节点的content信息转换成srcTab的路由表和from的路由器信息
		void ripPrintTable(map<string, struct RouterTab> &table, int &printNum, struct Host &host)
			打印主机信息
			
	
	dsdv.cpp
		模拟路由器本身，采用双线程的方式，一个线程用来监听udp端口，接受并且更新路由表。另一个线程用来定时刷新路由表并且发送。详细参见dsdv.cpp
			
路由表查找及更新的算法
	采用了dsdv的基本思路，通过seqNum来表示路由的最新状态。路径中的seqNum代表末节点的seqNum，其中如果seqNum是奇数，表示路由器正处在中间状态，他不能被更新，也不能更新其他路由器
	1.从硬盘刷新路由表：从硬盘读取新数据，比对路由器缓存的旧表（adjBuf），如果有变动，将自己的seqNum ++，同时将所有的到邻节点的路全部强制刷新成直接链接。并且更新路由表中所有通过邻节点的路径的cost。
	2.从邻节点更新路由表：首先存在4中情况：1.from seqNum 为偶数，host 为偶数；2.from 偶数，host奇数；3.from 奇数，host 奇数；4.from 奇数， host 偶数；
		对于1.from为中间状态，忽略
		2.host处于中间状态，忽略
		3.表示from和host正好是更新的边的两端点，更新到对应路径的seqNum，并且自身的seqNum ++
		4.又可以分为两种子情况，如果from的路由表中到自己的路线的seqNum为奇数，更行他的seqNum，此时中间状态完全结束，否则进行正常更新步骤
		由于硬盘刷新策略的设计是全部换成直连，所以当host路由表中发现from的seqNum大于路由表中保存的信息时，从buffer的邻节点表中更新所有到邻节点的路径。并且对路由表的路径也进行重新计算
		如果发现相等，对于所有路径进行更新：如果from中终点的seqNum较大，强制更新。否则重新计算路径的cost，以及判断是否改变路由路线从而获取最优值
		
	对于这个算法，只要有路线变化，他们的影响就会从变化的两个端点扩散出去，最差的情况可能就是整张路由表全部直间连接（相当与重建），然后逐步优化到最优，虽然效率一般，但是实现比较方便。
	
			
	

