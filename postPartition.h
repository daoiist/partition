//export LD_LIBRARY_PATH=/home/whj/internship/boost/lib:$LD_LIBRARY_PATH
#include <cstdio>
#include <cstdint>
#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <random>
#include <set>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <queue>
#include <boost/program_options.hpp>
#include <limits>
#include <chrono>
#include <fstream>
#include "./partition2.h"
#include "./dataset.h"
#include "./union_find.h"
#include "./universe.h"

class postPartition
{
public:
	postPartition()
	{
		//do nothing
	}
	
	//single aggregate
	postPartition(const std::string dir)
	{
		std::map<int,int> correlate1;
		std::vector<std::string> name1;
		Partition2 p1=run(dir,correlate1,name1);
		//p1*p2
		std::vector<std::string> joinname1=JoinName(name1);
		std::vector<std::set<int>> ID1=P2A(p1,joinname1,correlate1);
		//
		for(unsigned int i=0;i<size;i++)
			groupbyNameID.insert(std::pair<std::string,std::set<int>>(joinname1[i],ID1[i]));
	}
	//double aggregate
	postPartition(const std::string dir1,const std::string dir2)
	{
		std::map<int,int> correlate1;
		std::vector<std::string> name1;
		Partition2 p1=run(dir1,correlate1,name1);
		std::map<int,int> correlate2;
		std::vector<std::string> name2;
		Partition2 p2=run(dir2,correlate2,name2);
		//p1*p2
		Partition2 p12=p1 *p2;
		std::vector<std::string> joinname12=JoinName(name1,name2);
		std::vector<std::set<int>> ID12=P2A(p12,joinname12,correlate1);
		for(unsigned int i=0;i<size;i++)
			groupbyNameID.insert(std::pair<std::string,std::set<int>>(joinname12[i],ID12[i]));



	}
	//triple aggtegate
	postPartition(const std::string dir1,const std::string dir2,const std::string dir3)
	{
		std::map<int,int> correlate1;
		std::vector<std::string> name1;
		Partition2 p1=run(dir1,correlate1,name1);

		std::map<int,int> correlate2;
		std::vector<std::string> name2;
		Partition2 p2=run(dir2,correlate2,name2);

		std::map<int,int> correlate3;
		std::vector<std::string> name3;
		Partition2 p3=run(dir3,correlate3,name3);
		//p1*p2*p3
		Partition2 p123=p1 * p2 * p3;
		std::vector<std::string> joinname123=JoinName(name1,name2,name3);
		std::vector<std::set<int>> ID123=P2A(p123,joinname123,correlate1);
		for(unsigned int i=0;i<size;i++)
			groupbyNameID.insert(std::pair<std::string,std::set<int>>(joinname123[i],ID123[i]));





	}

	~postPartition()
	{
		
	}

	std::map<std::string,std::set<int>> getGB() const
	{
		return groupbyNameID;
	}
	
	unsigned int getSize() const { return size; }
	unsigned int getClass_nb() const { return class_nb; }
	Partition2 getPartition() const { return p; }

private:
	Partition2 run(std::string dir,std::map<int,int>& correlate,std::vector<std::string>& attribute)
	{
		std::vector<std::string> IDs;
		std::map<int,std::string> ID2atr,zero2sizeID2atr;
		
		Partition2 newPartition;
		extractFromLDB(dir,IDs,attribute);
		parseID(IDS,attribute,ID2atr,zero2sizeID2atr,correlate);
		newPartition=array2partition(zero2sizeID2atr);
		

		return newPartition;
	}

	int extractFromLMDB(std::string dir,std::vector<std::string>& IDs,std::vector<std::string>& attribute)
	{
		/* Fetch key/value pairs in a read-only transaction: */
		auto env = lmdb::env::create();
		env.set_mapsize(1UL * 1024UL * 1024UL * 1024UL); /* 1 GiB */
		env.open(dir, 0, 0664);
		auto rtxn = lmdb::txn::begin(env, nullptr, MDB_RDONLY);
		auto dbi = lmdb::dbi::open(rtxn, nullptr);
		auto cursor = lmdb::cursor::open(rtxn, dbi);
		std::string key, value;
	
		while (cursor.get(key, value, MDB_NEXT)) {
			//std::cout<<key.c_str()<<value.c_str()<<std::endl;
			attribute.push_back(key.c_str());
			IDs.push_back(value.c_str());
		}
	
		cursor.close();
		rtxn.abort();

		/* The enviroment is closed automatically. */
		class_nb = IDS.size();
		
		return EXIT_SUCCESS;
	}

	
	void parseID(std::vector<std::string> stringID,std::vector<std::string> atr,std::map<int,std::string>& ID2atr,std::map<int,std::string>& zero2sizeID2atr,std::map<int,int>& correlate)
	{
		std::string field;
		std::vector<int> originalID;
		for(int i=0;i<class_nb;i++)
		{
			int len=stringID[i].length();
			int index=len-1;
			if(stringID[i][len-1]==',')
				stringID[i][len-1]=='\n';
			std::istringstream is(stringID[i]);
			while(std::getline(is,field,','))
			{
				ID2atr.insert(std::pair<int,std::string>(stoi(field),atr[i]));
				originalID.push_back(stoi(field));
			}
		}
		size=ID2atr.size();
		for(int i=0;i<size;i++)
		{
			correlate.insert(std::pair<int,int>(i,originalID[i]));
			zero2sizeID2atr.insert(std::pair<int,std::string>(i,ID2atr[i]));
		}
	}
	
	Partition2 array2partition(const std::map<int,std::string>& zero2sizeID2atr)
	{
		int* T=new int[size];
		int* P=new int[size];
		memset(T,0,size);
		memset(P,0,size);
		for(int i=0;i<size;i++)
		{
			if(T[i])
				continue;
			for(int j=i;j<size;j++)
			{
				if(zero2sizeID2atr[i]==zero2sizeID2atr[j])
				{
					P[j]=i;
					T[j]=1;
				}
			}
		}
		return Partition2(size,P,"P");
	}

	std::vector<std::string> JoinName(std::vector<std::string> name)
	{
		std::vector<std::string> joinName;
		for(int i=0;i<size;i++)
		{	
			if(joinN.end()==std::find(joinName.begin(), joinName.end(), name)
			{
				//std::cout<<A1[i]+"|"+A2[i]<<std::endl;
				joinN.push_back(name[i]);
			}
		}
		//std::cout<<joinN.size()<<std::endl;
		return joinName;
	}

	std::vector<std::string> JoinName(std::vector<std::string> name1,std::vector<std::string> name2)
	{
		std::vector<std::string> joinName;
		for(int i=0;i<size;i++)
		{	
			if(joinN.end()==std::find(joinName.begin(), joinName.end(), name1[i]+"|"+name2[i])
			{
				//std::cout<<A1[i]+"|"+A2[i]<<std::endl;
				joinN.push_back(name1[i]+"|"+name2[i]);
			}
		}
		//std::cout<<joinN.size()<<std::endl;
		return joinName;
	}

	std::vector<std::string> JoinName(std::vector<std::string> name1,std::vector<std::string> name2,std::vector<std::string> name3)
	{
		std::vector<std::string> joinName;
		for(int i=0;i<size;i++)
		{	
			if(joinN.end()==std::find(joinName.begin(), joinName.end(), name1[i]+"|"+name2[i]+"|"+name3[i])
			{
				//std::cout<<A1[i]+"|"+A2[i]<<std::endl;
				joinN.push_back(name1[i]+"|"+name2[i]+"|"+name3[i]);
			}
		}
		//std::cout<<joinN.size()<<std::endl;
		return joinName;
	}

	std::vector<std::set<int>> P2A(const Partition2& P,std::vector<std::string> joinname,std::map<int,int> correlate)
	{	
		unsigned int M[size];
		auto& idx_cid = P.data().get<ClassId>();
		std::set<int> classID;
		for (auto cid_it = idx_cid.begin(); cid_it != idx_cid.end(); cid_it++)
		{
			M[cid_it -> first]=cid_it -> second -> Id();
			classID.insert(cid_it -> second -> Id());
			//std::cout<<cid_it -> first<<":"<<cid_it -> second -> Id()<<std::endl;
		}
		std::vector<std::set<int>> ID;
		
		for(std::set<int>::iterator it=classID.begin();it!=classID.end();it++)
		{
			std::set<int> temp;
			for(int j=0;j<dimension;j++)
			{
				if(M[j]==*it)
					temp.insert(correlate[j]);
			}
			ID.push_back(temp);
		}
		return ID;
	}



//private:
	Partition2 p;
	unsigned int size;
	unsigned int class_nb;
	std::map<std::string,std::set<int>> groupbyNameID;
	//std::vector<std::string> IDs,attribute;
	//std::map<int,std::string> ID2atr,zero2sizeID2atr;
	//std::map<int,int> correlate;
	
}












