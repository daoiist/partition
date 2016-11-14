// universe.h


#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <functional>
#include <unordered_set>
#include "anchor.h"


#ifndef UNIVERSE_H_INCLUDED
#define UNIVERSE_H_INCLUDED


using namespace boost::multi_index;


struct Id { };


class ClassId
{
public:
	ClassId(unsigned int i) : _id(i) { _next = this; }
	bool operator<(const ClassId& c) const { return _id < c._id; }
	unsigned int Id() { return _id; }
	ClassId * Next() { return _next; }
	void SetNext(ClassId * n) { _next = n; }
	unsigned int _id;
	ClassId * _next;
private:
	ClassId(const ClassId&);
};


typedef boost::shared_ptr<ClassId> PClassId;
typedef std::pair<unsigned int, PClassId> ObjPClassId;


unsigned int CID(const ObjPClassId& opid)
{
	return (opid.second) -> Id();
}


typedef multi_index_container<
	ObjPClassId,
	indexed_by<
		hashed_unique<
			tag<Id>,
			member<ObjPClassId, unsigned int, &ObjPClassId::first>
		>,
		hashed_non_unique<
			tag<ClassId>,
			global_fun<const ObjPClassId&,unsigned int, &CID>
		>
	>
> cid_container;




class RandomHashMap : public cid_container
{
public:
	/*void merge(unsigned int cid_1, unsigned int cid_2)
	{
		if (cid_1 == cid_2) return;
		if (cid_1 > cid_2) std::swap(cid_1, cid_2);

		// récupère l'id. de la classe cid_1 
		auto it = get<ClassId>().find(cid_1);
		// récupère tous les membres de la seconde classe
		auto it2 = get<ClassId>().equal_range(cid_2);
		while (it2.first != it2.second)
		{
			auto copie = it2.first++;
			if (copie != it2.second)
			{
				iterator glob = project<Id>(copie);
				get<ClassId>().replace(copie, *it);
			}
		}
	}*/
	/*void fragment(unsigned int cid)
	{
		// récupère tous les membres de la classe
		auto cid_range = get<ClassId>().equal_range(cid);
	
		while (cid_range.second != cid_range.first)
		{	
			auto copie = cid_range.first ++;
			if (copie != cid_range.second)
			{
				iterator glob = project<Id>(copie);
				auto position = get_absolute_position(cidc, glob);
				get<ClassId>().replace(copie, PClassId(new ClassId(position)));
			}
		}
	}
	void print(std::ostream& os, unsigned int cid)
	{
		auto cid_range = get<ClassId>().equal_range(cid);
	
		while (cid_range.second != cid_range.first)
		{	
			cid_container::iterator glob = project<Id>(cid_range.first ++);
			auto position = get_absolute_position(cidc, glob);
			os << position << " ";
		}
	}*/
};






struct bridge
{
	struct value_type
	{
		value_type(ClassId * first_, ClassId * second_):
		from(first_),to(second_){ real = from -> _id; }

		ClassId * from;
		ClassId * to;
		unsigned int real;
	};

	class hasher
	{
	public:
	    std::size_t operator()(value_type const& s) const 
	    {
		std::size_t seed = 0;
		boost::hash_combine(seed,s.from -> Id());
		boost::hash_combine(seed,s.to -> Id());
		return seed;
		// return h1 ^ h2; => particulièrement IMMONDE !!!
	    }
	};

	typedef multi_index_container<
		value_type,
		indexed_by<
			hashed_unique< member<value_type, unsigned int, &value_type::real>, boost::hash<unsigned int> >
			//hasher
			//boost::hash<value_type>
		>
	> type;
};



typedef std::pair<PClassId, PClassId> PairePClassId;
typedef multi_index_container<
	PairePClassId,
	indexed_by<
		hashed_unique<
			composite_key< PairePClassId,
				member< PairePClassId, PClassId, &PairePClassId::first>,
				member< PairePClassId, PClassId, &PairePClassId::second>
			> 
		>
	>
> zt_hashtable;



// safe version : tout existe au préalable
// renvoie 1 si l'appel entraîne la fusion de deux classes 
bool merge(cid_container& cidc, unsigned int cid_1, unsigned int cid_2)
{
	if (cid_1 == cid_2) return 0;
	if (cid_1 > cid_2) std::swap(cid_1, cid_2);
	
	auto it = cidc.get<ClassId>().find(cid_1);
	auto it2 = cidc.get<ClassId>().equal_range(cid_2);

	while (it2.first != it2.second)
	{
		// utile si invalidation des itérateurs
		auto copie = it2.first++;
		if (copie != it2.second)
			cidc.get<ClassId>().replace(copie, std::make_pair(copie -> first, it -> second));
	}

	return 1;
}


// scénario P = {{56}}, P.append({235})
// ajout de la classe 235 ou fusion avec classes existantes.
// merge(2,3) -> n'existe pas donc création : (2,2) puis (3,3) -> (2,3)
// merge(2,5) -> (2,2), (5,5) => (_,5) fusionne avec (_,2) !
// équivalent fusion deux arbres UF de profondeur 1 (pas de compression de chemins à faire)
// renvoie le nombre de classes créé (au sens de l'ajout à la partition) par l'opération
unsigned int merge_unsafe(cid_container& cidc, unsigned int cid_1, unsigned int cid_2)
{
	unsigned int cnt = 0;
	// on ne fait rien sauf si n'appartient pas au support
	if (cid_1 == cid_2)
	{
		auto it = cidc.get<Id>().find(cid_1);
		if (it == cidc.get<Id>().end())
		{
			cidc.insert(std::make_pair(cid_1, PClassId(new ClassId(cid_1))));
			++ cnt;
		}
		return cnt;
	}
	// identifiant de chaque classe à fusionner si elles existent
	cid_container::index<ClassId>::type::iterator it_cid, it_cid2;	

	// recherche si cid_1 est dans une classe, sinon la crée
	auto it = cidc.get<Id>().find(cid_1);
	if (it == cidc.get<Id>().end())
	{
		it_cid = cidc.project<1>((cidc.insert(std::make_pair(cid_1, PClassId(new ClassId(cid_1))))).first); // récup le cid
		++ cnt;
	}
	else	it_cid = cidc.project<1>(it);

	// recherche si cid_2 est dans une classe, sinon la crée
	auto it2 = cidc.get<Id>().find(cid_2);
	if (it2 == cidc.get<Id>().end())
	{
		it_cid2 = cidc.project<1>((cidc.insert(std::make_pair(cid_2, PClassId(new ClassId(cid_2))))).first); // récup le cid
		++ cnt;
	}
	else	it_cid2 = cidc.project<1>(it2);

	// détermine la classe à augmenter et son représentant
	if (it_cid -> second -> Id() == it_cid2 -> second -> Id()) return cnt;
	if (it_cid -> second -> Id() > it_cid2 -> second -> Id()) std::swap(it_cid,it_cid2);
	
	-- cnt;
	// fusion de la classe avec le nouveau représentant
	auto it2_r = cidc.get<ClassId>().equal_range(it_cid2 -> second -> Id());
	while (it2_r.first != it2_r.second)
	{
		// utile si invalidation des itérateurs
		auto copie = it2_r.first++;
		if (copie != it2_r.second)
			cidc.get<ClassId>().replace(copie, std::make_pair(copie -> first, it_cid -> second));
	}

	return cnt;
}


// renvoie le nombre de classes créé
unsigned int fragment(cid_container& cidc, unsigned int cid)
{
	unsigned int cnt = 0;
	auto cid_range = cidc.get<ClassId>().equal_range(cid);

	while (cid_range.second != cid_range.first)
	{	
		// utile si invalidation des itérateurs
		auto copie = cid_range.first ++;

		// on ne modifie seulement si l'identifiant n'est pas la classe elle-meme
		if (copie != cid_range.first && copie -> first != copie -> second -> Id())
		{
			++ cnt;
			cid_container::iterator glob = cidc.project<Id>(copie);
			//auto position = get_absolute_position(cidc, glob);
			cidc.get<ClassId>().replace(copie, std::make_pair(glob -> first, PClassId(new ClassId(glob -> first))));
		}
	}
	return cnt;
}



/*void print(std::ostream& os, cid_container& cidc, unsigned int cid)
{
	auto cid_range = cidc.get<ClassId>().equal_range(cid);
	
	while (cid_range.second != cid_range.first)
	{	
		cid_container::iterator glob = cidc.project<Id>(cid_range.first ++);
		auto position = get_absolute_position(cidc, glob);
		os << position << " ";
	}
}*/




namespace std
{
	template<>
	struct less< const PClassId* >
	{
		public:
		bool operator()(const PClassId* const  &lhs, const PClassId* const  &rhs) const
		{     
			return (*lhs) -> Id() < (*rhs) -> Id();
		}
	};
	template<>
	struct less< PClassId* >
	{
		public:
		bool operator()(PClassId* const  &lhs, PClassId* const  &rhs) const
		{     
			return (*lhs) -> Id() < (*rhs) -> Id();
		}
	};
	template<>
	struct less< ClassId *>
	{
		public:
		bool operator()(ClassId* const  &lhs, ClassId* const  &rhs) const
		{     
			return (lhs) -> Id() < (rhs) -> Id();
		}
	};
}

template <class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator univ_difference (InputIterator1 first1, InputIterator1 last1,
                                 InputIterator2 first2, InputIterator2 last2,
                                 OutputIterator result)
{
	while (first1!=last1 && first2!=last2)
	{
		if (first1 -> first < first2 -> first)
		{
			*result = *first1; ++result; ++first1;
		}
		else if (first2 -> first < first1 -> first)
			++first2;
		else
		{
			++first1; ++first2;
		}
	}
	return std::copy(first1,last1,result);
}


struct OnlyFirst
{
	public:
	bool operator()(ObjPClassId const  &lhs, ObjPClassId const  &rhs) const
	{     
		return (lhs).first < (rhs).first;
	}
};



template<typename T>
class implicit_reference_wrapper:public boost::reference_wrapper<T>
{
private:
  typedef boost::reference_wrapper<T> super;
public:
  implicit_reference_wrapper(T& t):super(t){}
};

typedef std::unordered_set<implicit_reference_wrapper<const ObjPClassId> > deck_view;



#endif
