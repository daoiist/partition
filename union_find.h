// union_find.h


#include <iostream>
#include "anchor.h"
#include "universe.h"

#ifndef UNION_FIND_H_INCLUDED
#define UNION_FIND_H_INCLUDED


static unsigned int PCOMP = 0;
static unsigned int max_iter=0;

// Prédicat d'égalité entre deux paires d'identifiants
struct Compare
{
	bool operator() (const Anchor<unsigned int> * lhs, const Anchor<unsigned int> * rhs) const
	{
		//bool test = (*lhs).second != 0 && (*rhs).second != 0;
		return lhs -> Id() < rhs -> Id() || (lhs -> Id() == rhs -> Id() && (lhs -> Next()) -> Id()  < (rhs -> Next()) -> Id());
	}
};
struct Uniq
{
	bool operator() (const Anchor<unsigned int> * lhs, const Anchor<unsigned int> * rhs) const
	{
		//bool test = (*lhs).second != 0 && (*rhs).second != 0;
		return lhs -> Id() < rhs -> Id();
	}
};


// hasheur pour les paires d'identifiants
struct Hash_anchor
{
	size_t operator () (const Anchor<unsigned int> * f) const
	{
		return std::hash<unsigned int>()(f -> Next() -> Id());
	}
};



struct UnionFind
{
	// Recherche la racine
	static inline Anchor<unsigned int> * RecSimpleFind(Anchor<unsigned int> * current, unsigned int iter = 0)
	{
		if (current -> Next() == current)
		{
			max_iter = (max_iter < iter ? iter : max_iter);
			return current;
		}
		else
		{
			++PCOMP;
			return RecSimpleFind(current -> Next(), iter + 1);
		}
	}
	// Méthode de Rem (Dijsktra) avec entrelacement des deux structures (si différentes) (procédurale)
	static inline void RecParallelFind(std::pair< Anchor<unsigned int> * , Anchor<unsigned int> * >& uv, unsigned int iter = 0)
	{
		//if (uv.first -> Next() != uv.first && uv.second -> Next() != uv.second)// aucune racine atteinte
		{
			if (*(uv.first -> Next()) < *(uv.second -> Next()))
			{
				//std::cout << "F(" << *(uv.second) << ") -> " << *(uv.first) << std::endl;
				Anchor<unsigned int> * swap = uv.second -> Next(); // save p(v)
				uv.second = swap; ++PCOMP; // v monte d'un cran
				uv.second -> SetNext(uv.first -> Next());
		                RecParallelFind(uv, ++iter);
			}
			else if (*(uv.first -> Next()) > *(uv.second -> Next()))
			{
				//std::cout << "F(" << *(uv.first) << ") -> " << *(uv.second) << std::endl;
				Anchor<unsigned int> * swap = uv.first -> Next(); // save p(u)
		                uv.first = swap; ++PCOMP; // u monte d'un cran
				uv.first -> SetNext(uv.second -> Next());
		                RecParallelFind(uv, ++iter);
		        }
			else return;
		}
		return;
	}
	// Méthode de Rem (Dijsktra) avec entrelacement des deux structures (si différentes) (procédurale)
	static inline void RecParallelFind2(std::pair< ClassId * , ClassId * >& uv, unsigned int iter = 0)
	{
		//if (uv.first -> Next() != uv.first && uv.second -> Next() != uv.second)// aucune racine atteinte
		{
			if (uv.first -> Next() -> Id() < uv.second -> Next() -> Id())
			{
				//std::cout << "F(" << *(uv.second) << ") -> " << *(uv.first) << std::endl;
				ClassId * swap = uv.second -> Next(); // save p(v)
				uv.second = swap; ++PCOMP; // v monte d'un cran
				uv.second -> _next = (uv.first -> Next());
		                RecParallelFind2(uv, ++iter);
			}
			else if (uv.first -> Next() -> Id() > uv.second -> Next() -> Id())
			{
				//std::cout << "F(" << *(uv.first) << ") -> " << *(uv.second) << std::endl;
				ClassId * swap = uv.first -> Next(); // save p(u)
		                uv.first = swap; ++PCOMP; // u monte d'un cran
				uv.first -> _next = (uv.second -> Next());
		                RecParallelFind2(uv, ++iter);
		        }
			else return;
		}
		return;
	}
	// Méthode de Rem sans l'entrelacement (procédurale)
	static inline void RecParallelFindWOSplicing(std::pair< Anchor<unsigned int> * , Anchor<unsigned int> * >& uv, unsigned int iter = 0)
	{
		// Condition a maintenir si on omet la compression (splicing) !
		if (uv.first -> Next() != uv.first && uv.second -> Next() != uv.second)
		{
		        if (*(uv.first -> Next()) < *(uv.second -> Next()))
		        {
		                uv.second = uv.second -> Next(); ++PCOMP; // v monte d'un cran
		                RecParallelFindWOSplicing(uv, ++iter);
		        }
		        else if (*(uv.first -> Next()) > *(uv.second -> Next()))
		        {
		                uv.first = uv.first -> Next(); ++PCOMP; // u monte d'un cran
		                RecParallelFindWOSplicing(uv, ++iter);
		        }
		}
		return;
	}
	static inline Anchor<unsigned int> * RecFind(Anchor<unsigned int> * current, unsigned int iter = 0)
	{
		if (current -> Next() == current)
		{
			max_iter = (max_iter < iter ? iter : max_iter);
			return current;
		}
		else
		{
			current -> Next() -> DecChildren();
			current -> SetNext(RecFind(current -> Next(), iter + 1));
			current -> Next() -> IncChildren();
			++PCOMP;
			return current -> Next();
		}
	}
	static inline ClassId * RecFind(ClassId * current, unsigned int iter = 0)
	{
		if (current -> Next() == current)
		{
			max_iter = (max_iter < iter ? iter : max_iter);
			return current;
		}
		else
		{
			current -> _next = (RecFind(current -> _next, iter + 1));
			++PCOMP;
			return current -> _next;
		}
	}
	// parcourt la structure et libère la mémoire
	static void Del(Anchor<unsigned int> * current)
	{
		if (current -> Next() == current)
		{
		        if (current -> Children() == 1) // no children
			{
				//std::cout << "del " << current -> Id() << std::endl;
				delete current;
			}
			//else std::cout << "no del " << current -> Id() << " : [" << current -> Children() << "]" << std::endl;
		        return;
		}
		else
		{
			if (current -> Children() == 0)
			{
				//std::cout << "del " << current -> Id() << " try " << current -> Next() -> Id()<< std::endl;
		               	current -> Next() -> DecChildren();
				Del(current -> Next());
				delete current;
			}
		        return;
		}
	}
};


#endif
