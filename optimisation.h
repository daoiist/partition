#ifndef OPTIMISATION_H
#define OPTIMISATION_H

#include "traits.h"
#include "transformation.h"



// patron du mécanisme d'application des lois internes de compositions
template<typename NodeType, typename IndicesType, unsigned int StartIndex = 0, typename enabled = void>
struct ReecritureAlgebrique;

// cas sans opération
template<typename T, typename IndicesType, unsigned int StartIndex>
struct ReecritureAlgebrique< Noeud<T, void, void>, IndicesType, StartIndex>
{
	typedef Noeud<T, void, void> TransformedNodeType;
	typedef IndicesType TransformedIndicesType;
};


// si noeud unaire
template<typename T, typename UnaryOp, typename IndicesType, unsigned int StartIndex>
struct ReecritureAlgebrique< Noeud<T, UnaryOp, void>, IndicesType, StartIndex>
{          
	typedef typename ReecritureAlgebrique<T, IndicesType, StartIndex>::TransformedNodeType ChildNodeType;
	typedef typename ReecritureAlgebrique<T, IndicesType, StartIndex>::TransformedIndicesType ChildIndicesType;

	typedef Noeud<ChildNodeType, UnaryOp, void> TransformedNodeType;
	typedef ChildIndicesType TransformedIndicesType;
};
    

/* Reorganisation d'un sous-arbre
//           Op
//          /  \
//         /    \
//        L      R
*/
template<typename L, typename OpType, typename R, typename IndicesType, unsigned int StartIndex>
struct ReecritureAlgebrique< Noeud<L, OpType, R>, IndicesType, StartIndex>
{
	// indice de la première opérande dans le sous-arbre enraciné en R
	static const int RightStart = StartIndex + L::TotalCount;

	/* optimisation (top-down) du SAD R
	//           \
	//           ROp
	//           / \
	//          /   \
	//        ...   ...
	*/
	typedef typename ReecritureAlgebrique<R, IndicesType, RightStart>::TransformedNodeType R0;
	typedef typename ReecritureAlgebrique<R, IndicesType, RightStart>::TransformedIndicesType I0;
	/* AST temporaire T0
	//           Op
	//          /  \
	//         /    \
	//        L     Rop
	//             /   \ 
	*/
	typedef Noeud<L, OpType, R0> T0;
	
	/* Reequilibrage a gauche ssi 
	// -- Op = Rop et A + (B + C) = (A + B) + C (associativité)
	// -- A + (B * C) = (A + B) * C	(modularité)
	// AST temporaire T0  =>  T1
	//        Op              Rop
	//       /  \            /   \
	//      /    \          /     \
	//     L     Rop       Op     RR
	//          /   \     /  \
	//         RL   RR   L    RL
	*/
	typedef typename RotationGauche<T0>::TransformedNodeType T1;
	typedef typename T1::Left LT1;
	typedef I0 I1;


	// Meme procédure sur le fils gauche L (ou le nouveau si la précédente a fonctionné LT1)
	typedef typename ReecritureAlgebrique<LT1, I1, StartIndex>::TransformedNodeType L2;
	typedef typename ReecritureAlgebrique<LT1, I1, StartIndex>::TransformedIndicesType I2;
	typedef Noeud<L2, OpType, typename T1::Right> T2;
	/* AST temporaire T2
	//           Op
	//          /  \
	//         /    \
	//       LOp     Rop
	//      /   \   /   \ 
	// Permutation des sous-arbres en fonction de la profondeur à droite (ie nb d'accumulateur temporaire) */
	typedef typename VerificatonPermutation<T2, I2, StartIndex>::TransformedNodeType T3;
	typedef typename VerificatonPermutation<T2, I2, StartIndex>::TransformedIndicesType I3;

	typedef typename VerificationReduction<T3, I3, StartIndex>::TransformedNodeType T4;
	typedef typename VerificationReduction<T3, I3, StartIndex>::TransformedIndicesType I4;

	typedef T4 TransformedNodeType;
	typedef I4 TransformedIndicesType;

};


// manipule l'arbre de syntaxe abstraite dans le but de minimiser le coût des opérations
template<typename NodeType>
struct GlobalOptimisation
{
	// liste statique des positions des opérateurs 
	typedef typename NodeType::Indices IndicesType;

	typedef typename ReecritureAlgebrique<NodeType, IndicesType, 0>::TransformedNodeType TransformedNodeType;
	typedef typename ReecritureAlgebrique<NodeType, IndicesType, 0>::TransformedIndicesType TransformedIndicesType;
};


/// \brief Counts the number of temporaries required a tree.
/// Value returns the number of temporaries required.
/// AsRhs gives the number assuming that the node is on the rhs of a 
/// binary node.
template<typename NodeType, typename enabled = void>
struct SADCount;

template<typename T>
struct SADCount<Noeud<T, void, void> >
{
	static const unsigned int Value = 0;
	static const unsigned int AsRhs = 0;
};

template<typename T, typename OpType>
struct SADCount< Noeud<T, OpType, void> >
{
	static const unsigned int Value = SADCount<T>::Value;
	static const unsigned int AsRhs = Value + 1;
};

template<typename L, typename OpType, typename R>
struct SADCount<Noeud<L, OpType, R>, typename boost::enable_if < EstTerminal<R> >::type >
{
	static const unsigned int Value = SADCount<L>::Value;
	static const unsigned int AsRhs = Value + 1;
};

template<typename L, typename OpType, typename R>
struct SADCount<Noeud<L, OpType, R>, typename boost::disable_if< EstTerminal<R> >::type >
{
	static const unsigned int Value = SADCount<L>::Value + SADCount<R>::Value + 1;
	static const unsigned int AsRhs = Value + 1;
};


#endif

