// logger.h



#define BOOST_ALL_DYN_LINK
#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED


#include <boost/thread/mutex.hpp>
#include <iostream>
#include "date.h"


#define LOG(x) (Log::Append(__FILE__, __FUNCTION__, __LINE__ , LogData<None>() << x))


struct None { };


template <typename List>
struct LogData
{
	List list;
};



template <typename Begin,typename Value>
LogData< std::pair<Begin,const Value &> > operator<<(LogData<Begin> begin,const Value &value)
{
	return {{begin.list,value}};
}



template <typename Begin,size_t n>
LogData< std::pair<Begin,const char *>> operator<<(LogData<Begin> begin,const char (&value)[n])
{
	return {{begin.list,value}};
}



inline void printList(std::ostream &,None)
{
}



template <typename Begin,typename Last>
void printList(std::ostream &os,const std::pair<Begin,Last> &data)
{
	printList(os,data.first);
	os << data.second;
}


class LogDestroyer;




class Log
{	
	friend class LogDestroyer;
public:
	template <typename List>
	static void Append(const char * file, const char * func, int line,const LogData<List>& data);
private:
	explicit Log()
	{
		log_file.open(log_filename, std::ios::out); log_file.close();
		log_file.open(log_filename, std::ios::app);
		//LOG("Ouverture du Logger");		
	}
	~Log();
	template <typename List>
	void do_log(const char *file, const char * func, int line,const LogData<List>& data)
	{
		auto tp = std::chrono::system_clock::now();
		using namespace date;
		log_file << tp << " : " << file << " [" << func << "] (" << line << "): ";
		printList(log_file, data.list);
		log_file << "\n"; log_file.flush();
	}
	static Log * L;
	static LogDestroyer _destroyer; 
	std::ofstream log_file;
	const char * log_filename = "trace.log";
};


class LogDestroyer
{
public:
	~LogDestroyer();
	void BindToLog(Log * l);
private:
	Log * _log;
};

Log * Log::L = 0;
LogDestroyer Log::_destroyer;



template <typename List>
void Log::Append(const char * file, const char * func, int line,const LogData<List>& data)
{
	if (0 == L)
	{
		L = new Log();
		_destroyer.BindToLog(L);
	}
	L -> do_log(file,func,line,data);
}
Log::~Log()
{
	log_file.close();
}


LogDestroyer::~LogDestroyer() { delete _log; }
void LogDestroyer::BindToLog(Log * l) { _log = l; }


//boost::mutex Log::mtx;


#endif

