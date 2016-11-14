// Partition2.h


#include <iostream>
#include <set>


#ifndef PARTITION2_H_INCLUDED
#define PARTITION2_H_INCLUDED


#define lund "\33[4m"
#define rund "\33[0m"


#include <unordered_map>
#include <algorithm>
#include <random>
#include "noeud.h"
#include "evaluation.h"
#include "universe.h"
#include "anchors_table.h"
#include "union_find.h"
#include "logger.h"


class Partition2
{
	class MyHash
	{
	public:
	    std::size_t operator()(std::pair<PClassId, PClassId> const& s) const 
	    {
		std::size_t seed = 0;
		boost::hash_combine(seed,s.first -> Id());
		boost::hash_combine(seed,s.second -> Id());
		return seed;
		// return h1 ^ h2; => particulièrement IMMONDE !!!
	    }
	};
	class MyHash2
	{
	public:
	    std::size_t operator()(std::pair<ClassId * , ClassId *> const& s) const 
	    {
		std::size_t seed = 0;
		boost::hash_combine(seed,s.first -> Id());
		boost::hash_combine(seed,s.second -> Id());
		return seed;
		// return h1 ^ h2; => particulièrement IMMONDE !!!
	    }
	};
public:
	Partition2() : _size(0), _classes_nb(0), _name("?")
	{
		LOG("+ Partition2" << " addr=" << this);
	}
	// construit une partition de taille donnée, remplit tout à zéro (partition top par défaut)
	Partition2(unsigned int size, std::string name): _size(size), _classes_nb(1), _name(name)
	{
		auto start = std::chrono::high_resolution_clock::now();

		_data.reserve(_size);
		for (unsigned int i = 0; i < _size ; ++i)
			_data.insert(std::make_pair(i, PClassId(new ClassId(0))));

		auto end = std::chrono::high_resolution_clock::now();
		int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		LOG("+ Partition2 (Top)" << " addr=" << this << " name=\"" << _name << "\" size=" << _size << " classes=" << _classes_nb << " feed=" << elapsed_ms << "ms");
	}
	// construit une partition depuis un tableau 
	Partition2(unsigned int size, unsigned int t[], std::string name): _size(size), _name(name)
	{
		auto start = std::chrono::high_resolution_clock::now();
		_data.reserve(_size);
		_classes_nb = 0;
		for (unsigned int i = 0 ; i < _size; ++i)
		{
			if (t[i] == i) // pas encore créé (id. classe)
			{
				_data.insert(std::make_pair(i,PClassId(new ClassId(t[i]))));
				++ _classes_nb;
			}
			else
			{
				auto couple = _data.get<ClassId>().find(t[i]);
				_data.insert(std::make_pair(i, (*couple).second));
			}
		}
		auto end = std::chrono::high_resolution_clock::now();
		int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		LOG("+ Partition2 (Arr)" << " addr=" << this << " name=\"" << _name << "\" size=" << _size << " classes=" << _classes_nb << " feed=" << elapsed_ms << "ms");
	}
	// construit une partition depuis une liste de classes
	Partition2(std::initializer_list<std::initializer_list<unsigned int>> nest_l) : _classes_nb(0), _name("?")
	{
		this -> _classes_nb = 0;
		auto start = std::chrono::high_resolution_clock::now();
		for (auto l : nest_l)
		{
			unsigned int min = -1;
			std::for_each(l.begin(), l.end(), [&min](unsigned int x) { min = (min < x ? min : x); });
			std::for_each(l.begin(), l.end(), [&] (unsigned int x) { this -> _classes_nb += merge_unsafe(this->_data, min, x); } );
			_size = _data.size();
		}
		auto end = std::chrono::high_resolution_clock::now();
		int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		LOG("+ Partition2 (LL)" << " addr=" << this << " name=\"" << _name << "\" size=" << _size << " classes=" << _classes_nb << " feed=" << elapsed_ms << "ms");
	}
	// ajoute une classe à la partition
	Partition2& add(std::initializer_list<unsigned int> l)
	{
		unsigned int min = -1;
		auto start = std::chrono::high_resolution_clock::now();

		std::for_each(l.begin(), l.end(), [&min](unsigned int x) { min = (min < x ? min : x); });
		std::for_each(l.begin(), l.end(), [&] (unsigned int x) { this -> _classes_nb += merge_unsafe(this->_data, min, x); } );
		_size = _data.size();

		auto end = std::chrono::high_resolution_clock::now();
		int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		LOG("+ Partition2 (Add)" << " addr=" << this << " name=\"" << _name << "\" size=" << _size << " classes=" << _classes_nb << " feed=" << elapsed_ms << "ms");
		return *this;
	}
	// ajoute un ens. de classe à la partition
	Partition2& add(std::initializer_list<std::initializer_list<unsigned int>> nest_l)
	{
		auto start = std::chrono::high_resolution_clock::now();
		for (auto l : nest_l)
		{
			unsigned int min = -1;
			std::for_each(l.begin(), l.end(), [&min](unsigned int x) { min = (min < x ? min : x); });
			std::for_each(l.begin(), l.end(), [&] (unsigned int x) { this -> _classes_nb += merge_unsafe(this->_data, min, x); } );
			_size = _data.size();
		}
		auto end = std::chrono::high_resolution_clock::now();
		int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		LOG("+ Partition2 (Add)" << " addr=" << this << " name=\"" << _name << "\" size=" << _size << " classes=" << _classes_nb << " feed=" << elapsed_ms << "ms");
		return *this;
	}
	// libère la mémoire allouée
	~Partition2()
	{
		LOG("- Partition2 " << " addr=" << this << " name=\"" << _name << "\"");
	}
	///// constucteur par recopie et = standard /////
	Partition2(const Partition2& P)
	{
		operator=(P);
	}
	Partition2& operator=(const Partition2& P)
	{
		// assurance contre l'aliasing
		if (&P != this)
		{
			_data.clear();
			set_param(P._size,P._name);
			_data.reserve(_size);
			_classes_nb = P._classes_nb;
			
			auto& idx = P._data.get<ClassId>();
			for (auto it = idx.begin(); it != idx.end(); ++it)
			{
				auto couple = _data.get<ClassId>().find(it -> second -> Id());
				if (couple == _data.get<ClassId>().end())
					_data.insert(std::make_pair(it -> first, PClassId(new ClassId(it -> second -> Id()))));
				else
					_data.insert(std::make_pair(it -> first, couple -> second));
			}
		}
		return *this;
	}
	/////////////////  MISC  ////////////////
	void Randomize(unsigned int sz, unsigned int nb_cl)
	{
		std::random_device rd;
		_size = sz;
		_data.clear();
		_data.reserve(_size);
		_classes_nb = nb_cl;

		auto start = std::chrono::high_resolution_clock::now();
		
		for (unsigned int k = 0; k < nb_cl ; ++k)
			_data.insert(std::make_pair(k, PClassId(new ClassId(k))));

		for (unsigned int i = 0; i < _size; ++i)
		{
			unsigned int k = rd() % nb_cl;
			if (k == i) continue;
			auto couple = _data.get<ClassId>().find(k);
			_data.insert(std::make_pair(i, couple -> second));
		}
		auto end = std::chrono::high_resolution_clock::now();
		int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		LOG("+ Randomize" << " size=" << _size << " classes=" << nb_cl  << " feed=" << elapsed_ms << "ms");
	}
	/////////// Op.s relationnels ///////////
	template <typename Expr1, typename Expr2>
	friend bool operator==(const Expr1& E, const Expr2& F)
	{
		typename Expr1::ResultType P;
		typename Expr2::ResultType Q;
		ExpressionEvaluator::Evaluate(E,P);
		ExpressionEvaluator::Evaluate(F,Q);
		return IsEqual(P,Q);
	}	
	template <typename Exp>
	friend bool operator==(const Exp& E, const Partition2& P)
	{
		typename Exp::ResultType Q;
		ExpressionEvaluator::Evaluate(E,Q);

		return IsEqual(Q,P);
	}
	template <typename Exp>
	friend bool operator==(const Partition2& P,const Exp& E) 
	{
		typename Exp::ResultType Q;
		ExpressionEvaluator::Evaluate(E,Q);

		return IsEqual(Q,P);
	}
	friend bool operator==(const Partition2& P, const Partition2& Q) 
	{
		return IsEqual(Q,P);
	}
	friend bool IsEqual(const Partition2& P, const Partition2& Q)
	{
		if (Q._data.size() != P._data.size()/* && P.classes_number() != Q.classes_number()*/)
			return false;

		auto& idx = P._data.get<Id>();
		for (auto it = idx.begin(); it != idx.end(); ++it)
		{
			// recherche id obj dans Q
			auto couple_q = Q._data.get<Id>().find(it -> first);
			if (it -> second -> Id() != couple_q -> second -> Id())
				return false;
		}
		return true;
	}
	friend bool operator!=(const Partition2& P, const Partition2& Q)
	{
		return !IsEqual(P,Q);
	}
	/////////////////////////////////////////
	template<typename Expression>
	Partition2(const Expression& E)
	{
		//std::cout << "EVALUATION DE L'AST" << std::endl;
		_name = "result";
		_size = 0;
		_classes_nb = 0;

		ExpressionEvaluator::Evaluate(E, *this);
	}
	template<typename Expression>
	Partition2& operator=(const Expression& E)
	{
		ExpressionEvaluator::Evaluate(E, *this);
		return * this;
	}
	/////////////////////////////////////////
	unsigned int size() const { return _size; }
	unsigned int classes_number() const { return _classes_nb; }
	std::string name() const { return _name; }
	friend std::ostream& operator<<(std::ostream& o, const Partition2& P)
	{		
		auto& idx_cid = P._data.get<ClassId>();
		auto last = idx_cid.begin() -> second -> Id();
		for (auto cid_it = idx_cid.begin(); cid_it != idx_cid.end(); cid_it++)
		{
			if (last != cid_it -> second -> Id()) o << "| ";
			if (cid_it -> first == cid_it -> second -> Id())
				o << lund << cid_it -> first << rund << " ";
			else
				o << cid_it -> first << " ";
			
			last = cid_it -> second -> Id();
		}
		o << std::endl;
		return o;
	}
	size_t hash() const
	{
		size_t total = 0;
		return total;
	}
	cid_container& data() { return _data; }
	cid_container const& data() const { return _data; }
	///////////////////////////////////////////////////////////////
	/// met en oeuvre les fonctionnalités de l'évaluation générique
	///////////////////////////////////////////////////////////////
	friend Partition2 Add(const Partition2& P, const Partition2& Q)
	{
		Partition2 * tmp = new Partition2(P._size,"res");
		Join(P, Q, *tmp, P._size);
		return * tmp;
	}

	friend void AddEqual(Partition2& accumulator, const Partition2& Q)
	{
		Join(accumulator, Q, accumulator, accumulator._size);
	}
	friend void Add(Partition2& accumulator, const Partition2& P, const Partition2& Q)
	{
		accumulator = P;
		Join(P, Q, accumulator, P._size);
	}
	///////////////////////////////////////////////////////////////
	friend Partition2 Sub(const Partition2& P, const Partition2& Q)
	{
		Partition2 * tmp = new Partition2(P);
		Less(P, Q, *tmp, P._size);
		return * tmp;
	}
	friend void SubEqual(Partition2& accumulator, const Partition2& Q)
	{
		Less(accumulator, Q, accumulator, accumulator._size);
	}
	// On force ici l'accumulateur à être P, étant donné que le résultat dépend intégralement de P
	friend void Sub(Partition2& accumulator, const Partition2& P, const Partition2& Q)
	{
		accumulator = P;
		Less(P, Q, accumulator, P._size);
	}
	///////////////////////////////////////////////////////////////
	friend Partition2 Mul(const Partition2& P, const Partition2& Q)
	{
		Partition2 * tmp = new Partition2(P._size,"res");
		Meet(P, Q, *tmp, P._size);
		return * tmp;
	}

	friend void MulEqual(Partition2& accumulator, const Partition2& Q)
	{
		Meet(accumulator, Q, accumulator, accumulator._size);
	}
	friend void Mul(Partition2& accumulator, const Partition2& P, const Partition2& Q)
	{
		Meet(P, Q, accumulator, P._size);
	}
private:
	// réalise Acc = P \meet Q
	static void Meet(const Partition2& P, const Partition2& Q, Partition2& Acc, unsigned int SZ)
	{	
		LOG("MEET(" << P.name() << "," << Q.name() << ")" << " accu=" << (&P == &Acc ? "0" : "1"));

		auto start = std::chrono::high_resolution_clock::now();
		std::unordered_map<std::pair<PClassId , PClassId>, ObjPClassId, MyHash> zt;

		if (&P != &Acc)
		{
			Acc._data.clear();
			Acc._size = P._size;
			Acc._data.reserve(Acc._size);
		}

		unsigned int cnt = 0;

		auto& idx = &P != &Acc ? P._data.get<Id>() : Acc._data.get<Id>();
		size_t i = 0;
		zt.reserve(P._classes_nb * Q._classes_nb * (4/3)); //max_load_factor(0.75);
		for (auto pit = idx.begin(); pit != idx.end(); ++pit, ++i)
		{
			auto qit = Q._data.find(pit -> first);

			// si ens. support différent : on ne retient que l'intersection du support
			if (qit == Q._data.end() || pit == idx.end())
			{
				if (&P == &Acc)
					Acc._data.erase(pit);
				continue;
			}
	
			auto couple = std::make_pair(pit -> second, qit -> second);
			auto got = zt.find(couple);

			ObjPClassId act_cid;

			bool is_new = !(got != zt.end());
			size_t last = 0;// = act_cid.first;
			bool need_update = 0;

			//if (i % (P.data().size() / 20) == 0) std::cout << i << " " << zt.size() << " "<<  zt.load_factor() << " " << zt.bucket_count()<< std::endl;

			if (is_new) // première intersection (z,t) 
			{
				ObjPClassId anc(std::make_pair(pit -> first, PClassId(new ClassId(pit -> first))));	
				zt[couple] = anc;
				act_cid = anc;
			}
			else // nouvelle intersection (z,t) 
			{
				act_cid = got -> second;
				last = act_cid.second -> Id();
				if (act_cid.second -> Id() > pit -> first) // "meilleure" intersection
				{
					ObjPClassId anc(std::make_pair(pit -> first, PClassId(new ClassId(pit -> first))));	
					zt[couple] = anc;
					act_cid = anc;
					need_update = 1;
					cnt ++;
				}
			}

			if (&P != &Acc) // externe
				Acc._data.insert(std::make_pair(pit -> first, act_cid.second));
			else // alt. version (interne)
				Acc._data.replace(pit, std::make_pair(pit -> first, act_cid.second));
			// si l'ancien id. de classe n'était pas le bon, on doit le changer avec le nouveau (identique à la fusion de classes UF)			
			if (!is_new && need_update) merge_unsafe(Acc._data, pit -> first, last);
		}

		Acc._classes_nb = zt.size();
		auto end = std::chrono::high_resolution_clock::now();
		int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		Acc._name = "("+P._name + "*" + Q._name+")";
		LOG("MEET addr=" << &Acc << " name=\"" << Acc._name << " size=" << Acc._size 
			<< " classes=" << Acc._classes_nb 
			<< " cl.prev.=" << P._classes_nb * Q._classes_nb 
			<< " unsafe=" << cnt 
			<< "\" all=" << elapsed_ms << "ms");
	}
	// réalise Acc = P \join Q
	static void Join(const Partition2& P, const Partition2& Q, Partition2& Acc, unsigned int SZ)
	{
		LOG("JOIN(" << P.name() << "," << Q.name() << ")" << " accu=" << (&P == &Acc ? "0" : "1"));
		auto start = std::chrono::high_resolution_clock::now();
		bridge::type b;
		std::set<ClassId *> anchors;

		// si support différent : classes manquantes dans P
		for (auto it = Q._data.begin(); it != Q._data.end(); ++it)
			if (Acc._data.find(it -> first) == Acc._data.end())
				Acc._data.insert(std::make_pair(it -> first, PClassId(new ClassId(it -> first))));
		
		// Pour tout P selon l'ordre défini sur les classes
		auto& idx_cid = Acc._data.get<ClassId>(); // des erreurs de lecture dans la boucle de merge(.,.,.) lorsque get<Id> ?!!!
		for (auto cid_it = idx_cid.begin(); cid_it != idx_cid.end(); cid_it++)
		{
			PClassId the_x = cid_it -> second;			
			size_t one_x = cid_it -> first;

			// recherche de l'obj id dans Q
			auto it = Q._data.get<Id>().find(one_x);	

			// si pas trouve on passe au suivant dans P
			if (it == Q._data.get<Id>().end())
				continue;

			PClassId the_u = it -> second;
		        ClassId * the_y = 0;

			auto bit = b.find(the_u -> Id()); // on cherche dans la table un pont
			if (bit == b.end())
			{
				auto it_u = Q._data.get<ClassId>().equal_range(the_u -> Id());
				while (it_u.first != it_u.second && the_y == 0)
				{
					size_t one_v = it_u.first -> first; it_u.first++;
					if (one_x == one_v) continue;

					auto it = Acc._data.get<Id>().find(one_v);
					the_y = (it -> second).get();
				}
			}
			else
				the_y = (*bit).to;

			if (the_y == 0) continue;

			std::pair< ClassId *, ClassId *> uv;
			
			uv.first = the_x.get(); uv.second = the_y;
			anchors.insert(the_x.get());

                        UnionFind::RecParallelFind2(uv);
			ClassId * px = uv.first -> Next();
			ClassId * py = uv.second -> Next();
                        ClassId * pu = the_u.get();

			if (px -> Id() == py -> Id() && b.find(the_u -> Id()) == b.end())
				b.insert(bridge::value_type(pu, px));
		}
		
		//auto& idx = Acc._data.get<Id>();
		for (auto anc : anchors)
		{
			/*auto nw = */UnionFind::RecFind((anc));
		}
		for (auto anc : anchors)
		{
			if (anc -> Id() != anc -> Next() -> Id())
				Acc._classes_nb -= merge(Acc._data, anc -> Id(), anc -> Next() -> Id());
		}

		// probleme: a faire en deux fois sinon valgrind gueule ...
		/*for (auto itp = idx.begin(); itp != idx.end(); ++itp)
		{
			//if (itp -> second -> Id() != itp -> second -> _next -> Id())
			//	Acc._classes_nb -= merge(Acc._data, itp -> second -> Id(), itp -> second -> _next -> Id());

			itp -> second -> _next = (itp -> second).get();
		}*/

		//std::cout << anchors.size() << std::endl;

		auto end = std::chrono::high_resolution_clock::now();
		int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		Acc._name = "("+P._name + "+" +Q._name+")";
		LOG("JOIN addr=" << &Acc << " name=\"" << Acc._name << " size=" << Acc._size
			<< " classes=" << Acc._classes_nb
			<< "\" all=" << elapsed_ms<<"ms");
	}
	// réalise Acc = P - Q
	static void Less(const Partition2& P, const Partition2& Q, Partition2& Acc, unsigned int SZ)
	{
		LOG("LESS(" << P.name() << "," << Q.name() << ")" << " accu=" << (&P == &Acc ? "0" : "1"));

		auto start = std::chrono::high_resolution_clock::now();

		std::set<const PClassId *> anchors;
		std::set<const PClassId *> to_keep, to_frag;
		std::set<PClassId> z_set;
		zt_hashtable zt_ht;
		
		for (unsigned int i = 0; i < Acc._size; ++i)
		{
			auto pit = Acc._data.get<Id>().find(i), qit = Q._data.get<Id>().find(i);
			auto got = zt_ht.find(boost::make_tuple(pit -> second, qit -> second));
			auto got2 = z_set.find(pit -> second);

			anchors.insert(&(pit -> second));
			
			if (got == zt_ht.end()) // si nouvelle intersection (z,t)
			{
				if (got2 == z_set.end()) // nouveau (z,_) (première fois)
				{
					// exactement la meme classe dans Q
					if (pit -> second -> Id() == qit -> second -> Id() && (pit -> second).use_count() == (qit -> second).use_count())
						to_keep.insert(&(pit -> second));

					zt_ht.insert(std::make_pair(pit -> second, qit -> second));
					z_set.insert(pit -> second);
				}
				else // si seconde rencontre (z,t_1), (z,t_2), on garde
					to_keep.insert(&(pit -> second));
			}
		}
		std::set_difference(anchors.begin(), anchors.end(), to_keep.begin(), to_keep.end(), std::inserter(to_frag, to_frag.end()), std::less<const PClassId *>());
		
		for (auto tk : to_frag)
			Acc._classes_nb += fragment(Acc._data, (*tk) -> Id());
		
		auto end = std::chrono::high_resolution_clock::now();
		int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		Acc._name = "("+P._name + "-" + Q._name+")";
		LOG("LESS addr=" << &Acc << " name=\"" << Acc._name << " size=" << Acc._size
			<< " classes=" << Acc._classes_nb
			<< "\" all=" << elapsed_ms<<"ms");
	}
	void set_param(unsigned int sz, std::string n)
	{
		_size = sz; _name = n;
	}
	unsigned int _size;
	unsigned int _classes_nb;
	cid_container _data;
	std::string _name;
};



DECLARE_COMMUTATIVE(Partition2, AddOp);
DECLARE_ASSOCIATIVE(Partition2, AddOp);
DECLARE_IDEMPOTENT(Partition2, AddOp);

DECLARE_COMMUTATIVE(Partition2, MulOp);
DECLARE_ASSOCIATIVE(Partition2, MulOp);
DECLARE_IDEMPOTENT(Partition2, MulOp);


template<typename NodeType, typename Indices, unsigned int StartIndex>
struct GenerateTemporaryResult<Partition2, NodeType, Indices, StartIndex>
{
	static const unsigned int MappedIndex = OPERANDS_POS_FROM_INDEX(Indices,StartIndex);

	template<typename ArgVectorType>
	static Partition2 Apply(const ArgVectorType& subtree)
	{
		const Partition2& m = GET_OPERAND(MappedIndex,subtree);
		std::string s = "A" + m.name();
		return Partition2(0,s);	// A DEBUG : il confond Partition::Partition<Expression = char[n]> avec P::P(std::string)...
	}
};


Noeud<Noeud<Partition2 >, AddOp, Noeud<Partition2 > > operator+(const Partition2 & lhs, const Partition2 & rhs)
{ 
	return CreateBinaryExpression<AddOp,Partition2,Partition2>(lhs, rhs); 
}
Noeud<Noeud<Partition2 >, SubOp, Noeud<Partition2 > > operator-(const Partition2 & lhs, const Partition2 & rhs)
{ 
	return CreateBinaryExpression<SubOp,Partition2,Partition2>(lhs, rhs); 
}
Noeud<Noeud<Partition2 >, MulOp, Noeud<Partition2 > > operator*(const Partition2 & lhs, const Partition2 & rhs)
{ 
	return CreateBinaryExpression<MulOp,Partition2,Partition2>(lhs, rhs); 
}


#endif




