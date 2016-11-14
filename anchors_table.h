// anchors_table.h


#ifndef ANCHORS_TABLE_H_INCLUDED
#define ANCHORS_TABLE_H_INCLUDED


#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>



using boost::multi_index_container;
using namespace boost::multi_index;

struct anchors_table
{
	struct value_type
	{
		value_type(const PAnchor& first_,const PAnchor& second_):
		first(first_),second(second_){}

		PAnchor first;
		PAnchor second;
	};

	typedef multi_index_container<
		value_type,
		indexed_by<
			ordered_non_unique< member<value_type, PAnchor, &value_type::first> >,
			ordered_unique<
				composite_key<value_type,
					member<value_type, PAnchor, &value_type::first>,
					member<value_type, PAnchor, &value_type::second>
					> 
				>
			>
		> type;
};


#endif


