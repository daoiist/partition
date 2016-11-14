// dataset.h


#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>



#ifndef DATASET_H_INCLUDED
#define DATASET_H_INCLUDED


#include "table.h"


class Dataset
{
public:
	Dataset(const std::string& ap) : absolute_dir_path(ap)
	{
		DIR *dp;
		struct dirent *dirp;
		if((dp = opendir(ap.c_str())) == 0)
		{
			std::cout << "Error(" << errno << ") opening " << ap << std::endl;
		}

		while ((dirp = readdir(dp)) != 0)
		{
			std::string fn(dirp->d_name);
			std::string::size_type t = fn.find_last_of(".");
			if (fn.substr(t+1,fn.size()) == "csv")
			{
				data_filenames.push_back(fn);
				Table * t = new Table(*this, absolute_dir_path+"/"+fn);
				data_tables.emplace_back(t);
			}
		}
		closedir(dp);
	}
	~Dataset()
	{
		for (auto it = data_tables.begin(); it != data_tables.end(); ++it)
		{
			Table * t = *it;
			delete t;
		}
	}
	std::vector<Table *>& get_tables()
	{
		return data_tables;
	}
	
private:
	Dataset();
	Dataset(const Dataset& d);
	std::string 			absolute_dir_path;	// chemin du répertoire
	std::vector<std::string>	data_filenames;		// liste des fichiers recensée dans le répertoire
	std::vector<Table *>		data_tables;
};


#endif
