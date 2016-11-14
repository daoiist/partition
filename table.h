// table.h


#include <iostream>
#include <fstream>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_numeric.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>


#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include "dataset.h"
//#include "partition.h"
#include "partition2.h"

class Dataset;


class Table
{
public:
	Table(const Dataset& d, const std::string s) : dataset(d), absolute_filename(s), data_tuples(0), row_size(0), col_size(0), allocated_bytes(0)
	{ 
		if (!exists(absolute_filename))
			std::cout << "!> le fichier \"" << absolute_filename << "\" n'existe pas." << std::endl; // lever une exception serait judicieux

		// recherche header et schema
		std::ifstream fl_src;
		fl_src.open(absolute_filename, std::ifstream::in);
		
		unsigned int lines = 0;
		has_header = true;
		has_schema = true;
		while (fl_src.good())
		{
			std::string sentence;
			fl_src >> sentence;
			if (all_numeric(sentence))
			{
				if (lines == 0) has_header = false;
				if (lines == 1) has_schema = false;
			}
			else if (lines == 0 || lines == 1) // les colonnes
			{
				cols<std::string::iterator> parser;
				boost::spirit::qi::parse(sentence.begin(), sentence.end(), parser, columns);
			}
			++ lines;
		}
		row_size = lines - (has_header + has_schema) - 1;

		fl_src.close();

		load_data();
	}
	~Table()
	{
		//std::cout << "~Table " << this << std::endl;
		for (unsigned int i = 0; i < row_size; ++i)
			delete[] data_tuples[i];
		delete[] data_tuples;
		//std::cout << "~Table free'd" << std::endl;
	}
	void load_data()
	{
		unsigned int last_cs = 0;
		
		std::ifstream fl_src;
		fl_src.open(absolute_filename, std::ifstream::in);

		std::set<unsigned int> classes;

		unsigned int obs = 0;
		unsigned int lines = 0;

		while (fl_src.good() && obs < row_size)
		{
			std::string sentence;
			fl_src >> sentence;

			if (lines >= (has_header + has_schema))
			{
				tuple<std::string::iterator> parser;
				std::vector<double> temp;
				bool result = boost::spirit::qi::parse(sentence.begin(), sentence.end(), parser, temp);
				
				if (result == 0) {
					std::cout << " PROBLEME SUR LES DONNEES !!! " << lines << " " << obs << std::endl;
					std::cout << " >> \"" << sentence << "\"" << std::endl;}
				else
				{
					if (col_size == 0) last_cs = col_size = temp.size();
					if (data_tuples == 0) 
					{
						data_tuples = new double*[row_size];
						allocated_bytes += row_size * sizeof(double*);
					}
					
					if (col_size != last_cs)
					{
						std::cout << " COL SIZE VARIANT !!! " << lines << " " << obs << " : " << col_size << " " << last_cs << std::endl;
						std::cout << " >> \"" << sentence << "\"" << std::endl;
					}
					else
					{
						data_tuples[obs] = new double[col_size]; allocated_bytes += col_size * sizeof(double);
						memcpy(data_tuples[obs],&(temp[0]), sizeof(double)*col_size);
						classes.insert(data_tuples[obs][col_size-1]);
						//std::cout << obs << " > " << data_tuples[obs][0] << " " << data_tuples[obs][1] << std::endl;
					}
				}
				last_cs = temp.size();
				++ obs;
			}
			++ lines;
		}
		classes_num = classes.size();
		fl_src.close();
	}
	const std::string& get_name() const
	{
		 return absolute_filename;
	}
	unsigned int get_row_size() const
	{
		 return row_size;
	}
	unsigned int get_col_size() const
	{ 
		return col_size;
	}
	unsigned int get_classes_num() const
	{ 
		return classes_num;
	}
	double ** tuples()
	{
		return data_tuples;
	}
	friend std::ostream& operator<<(std::ostream& o, const Table& t)
	{
		o << "Table : " << t.absolute_filename << std::endl 
				<< "\thas_header = " << t.has_header << std::endl 
				<< "\thas_schema = " << t.has_schema << std::endl
				<< "\t row * col = " << t.row_size << " x " << t.col_size<< std::endl
				<< "\tsize = " << t.allocated_bytes << std::endl
				<< "\tclasses = " << t.classes_num;
		return o;
	}
private:
	template <typename Iterator>
	struct cols : boost::spirit::qi::grammar<Iterator, std::vector<std::string>()>
	{
		cols() : cols::base_type(query)
		{
			using boost::spirit::qi::char_;
			using boost::spirit::qi::lit;
			query =  key >> *(lit(',') >> key);
			key   =  char_("a-zA-Z_.") >> *char_("a-zA-Z_0-9.");
		}
		boost::spirit::qi::rule<Iterator, std::vector<std::string>()> query;
		boost::spirit::qi::rule<Iterator, std::string()> key;
	};
	template <typename Iterator>
	struct tuple : boost::spirit::qi::grammar<Iterator, std::vector<double>()>
	{
		tuple() : tuple::base_type(query)
		{
			using boost::spirit::qi::double_;
			using boost::spirit::qi::lit;
			query =  double_ >> *(lit(',') >> double_);
		}
		boost::spirit::qi::rule<Iterator, std::vector<double>()> query;
	};
	inline bool exists(const std::string& name)
	{
		struct stat buffer; 
		return (stat(name.c_str(), &buffer) == 0);
	}
	bool all_numeric(std::string const& str)
	{
		using boost::spirit::qi::char_;
		using boost::spirit::qi::double_;
    		using boost::spirit::ascii::space;
		std::string::const_iterator first(str.begin()), last(str.end());
   		return boost::spirit::qi::parse(first, last, 
							double_ >> 
							*(char_(',') >> double_))
			&& first == last;
	}
	const Dataset& 		dataset;
	const std::string 	absolute_filename;
	bool			has_schema;		// si typage des colonnes
	bool			has_header;		// si en-tête des colonnes
	std::vector<std::string>columns;		// noms des colonnes
	double **		data_tuples;		// données
	unsigned int		row_size;		// nombre de tuples
	unsigned int		col_size;		// nombre de colonnes
	unsigned int		classes_num;		// nombre de classes
	unsigned int		class_column;		// classe col.id.
	unsigned int		allocated_bytes;	// comptabilité
};



#endif
