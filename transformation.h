#ifndef TRANSFORMATION_H_INCLUDED
#define TRANSFORMATION_H_INCLUDED


#include "traits.h"
#include "noeud.h"
#include <boost/type_traits.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/erase.hpp>



/// \brief Utility metafunction to swap indices.
///
/// For a given sequence, this metafunction swaps the segment [start, partition) with 
/// [partition, end).  
///
/// For example, with the sequence 0, 1, 2, 3, 4, 5, start = 2, partition =4, end = 6, the 
/// result of this metafunction is the sequence 0, 1, 4, 5, 2, 3.
template<typename InputSequence, unsigned int start, unsigned int partition, unsigned int end>
struct Swap
{
	typedef typename boost::mpl::begin<InputSequence>::type Begin;
	typedef Begin PrefixStart;
	typedef typename boost::mpl::advance_c< Begin, start >::type PrefixEnd;

	typedef PrefixEnd FirstPartitionStart;
	typedef typename boost::mpl::advance_c< Begin, partition>::type FirstPartitionEnd;

	typedef FirstPartitionEnd SecondPartitionStart;
	typedef typename boost::mpl::advance_c<Begin, end>::type SecondPartitionEnd;

	typedef SecondPartitionEnd SuffixStart;
	typedef typename boost::mpl::end<InputSequence>::type SuffixEnd;

	typedef typename boost::mpl::iterator_range<PrefixStart, PrefixEnd>::type Prefix;
	typedef typename boost::mpl::iterator_range<FirstPartitionStart, FirstPartitionEnd>::type FirstPartition;
	typedef typename boost::mpl::iterator_range<SecondPartitionStart, SecondPartitionEnd>::type SecondPartition;
	typedef typename boost::mpl::iterator_range<SuffixStart, SuffixEnd>::type Suffix;

	typedef typename boost::mpl::joint_view<Prefix, SecondPartition>::type View1;
	typedef typename boost::mpl::joint_view<FirstPartition, Suffix>::type View2;
	typedef typename boost::mpl::joint_view<View1, View2>::type type;

};


template<typename InputSequence, unsigned int pos, unsigned int end>
struct Erase
{
	typedef typename boost::mpl::begin<InputSequence>::type Begin;
	typedef Begin PrefixStart;
	typedef typename boost::mpl::advance_c< Begin, pos >::type PrefixEnd;
	typedef typename boost::mpl::iterator_range<PrefixStart, PrefixEnd>::type Prefix;
	
	//BOOST_MPL_ASSERT_RELATION( boost::mpl::size<Prefix>::value, ==, 1);

	typedef typename boost::mpl::advance_c< Begin, end>::type SuffixStart;
	typedef typename boost::mpl::end<InputSequence>::type SuffixEnd;
	typedef typename boost::mpl::iterator_range<SuffixStart, SuffixEnd>::type Suffix;	

	//BOOST_MPL_ASSERT_RELATION( boost::mpl::size<Suffix>::value, ==, 0);
	typedef typename boost::mpl::joint_view<Prefix, Suffix>::type type;


	//typedef typename boost::mpl::find< Begin, Pos >::type I;
	//typedef typename boost::mpl::advance_c< Begin, end >::type End;
	//typedef typename boost::mpl::erase<InputSequence, I>::type type;
	//typedef InputSequence type;
};


/// \brief Performs a commutative transform on a node if the operator is commutative.
template<typename NodeType, typename IndicesType, unsigned int IndexStart, typename enabled=void>
struct Permuter
{
	typedef NodeType TransformedNodeType;
	typedef IndicesType TransformedIndicesType;
};

template<typename Left, typename Op, typename Right, typename IndicesType, unsigned int IndexStart>
struct Permuter< Noeud<Left, Op, Right>, IndicesType, IndexStart,
		typename boost::enable_if<
				EstCommutatif<typename Left::ResultType, Op, typename Right::ResultType> 
				>::type>
{
	typedef Noeud<Left, Op, Right> BaseNode;
	typedef Noeud<Right, Op, Left> TransformedNodeType;

	static const unsigned int partition = IndexStart + Left::TotalCount; 
	static const unsigned int end = IndexStart + Left::TotalCount + Right::TotalCount;
	typedef typename Swap<IndicesType, IndexStart, partition, end>::type TransformedIndicesType;    
};



// Performs a commutative transform only if the transformation will remove a temporary.
template<typename NodeType, typename IndicesType, unsigned int StartIndex, typename enabled = void>
struct VerificatonPermutation
{
	typedef NodeType TransformedNodeType;
	typedef IndicesType TransformedIndicesType;
};

// Expressions such as A + (BC), where a commutative transform will remove a temporary.
template<typename LhsType, typename OpType, typename RhsType, typename IndicesType, unsigned int StartIndex>
struct VerificatonPermutation< Noeud<LhsType, OpType, RhsType>, IndicesType, StartIndex,
		typename boost::enable_if< boost::mpl::and_<
						EstCommutatif<typename LhsType::ResultType, OpType, typename RhsType::ResultType>,
						boost::is_same<typename LhsType::OpType, void>,
						boost::mpl::not_<boost::is_same<typename RhsType::OpType, void> >
						>
					>::type>
{
	typedef Noeud<LhsType, OpType, RhsType> NodeType;
	typedef typename Permuter<NodeType, IndicesType, StartIndex>::TransformedNodeType TransformedNodeType;
	typedef typename Permuter<NodeType, IndicesType, StartIndex>::TransformedIndicesType TransformedIndicesType;
};


/// \brief Transforms the given tree using an associative transform if appropriate.
///
/// Given a tree of the form A Op (B Op C), if Op is associative, this metafunction
/// transforms it to (A Op B) Op C.
template<typename NodeType, typename enabled = void>
struct RotationGauche
{
	typedef NodeType TransformedNodeType;
};

template<typename LhsNodeType, typename OpType, typename R1, typename ROp, typename R2>
struct RotationGauche< Noeud<LhsNodeType, OpType, Noeud<R1, ROp, R2> >,
		typename boost::enable_if<
			EstAssociatif<
				typename LhsNodeType::ResultType,
				OpType,
				typename R1::ResultType,
				ROp
			>
		>::type>
{
	typedef Noeud<Noeud<LhsNodeType, OpType, R1>, ROp, R2> TransformedNodeType;
};

/// \brief Transforms the given tree using an associative transform if appropriate.
///
/// Given a tree of the form (A Op B) Op C, if Op is associative, this metafunction
/// transforms it to A Op (B Op C).
template<typename NodeType, typename enabled = void>
struct RotationDroite
{
	typedef NodeType TransformedNodeType;
};

template<typename L1, typename LOp, typename L2, typename OpType, typename RhsNodeType>
struct RotationDroite<Noeud<Noeud<L1, LOp, L2>, OpType, RhsNodeType >,
		typename boost::enable_if<
			EstAssociatif<
				typename L1::ResultType,
				LOp,
				typename L2::ResultType,
				OpType
			>
		>::type>
{
	typedef Noeud<L1, LOp, Noeud<L2, OpType, RhsNodeType> > TransformedNodeType;
};




/// \brief Réduit un sous-arbre ssi même opérande et opérateur idempotent
/// Sachant A Op A, on renvoie A 
///
template <typename NodeType, typename IndicesType, unsigned int IndexStart, typename enabled=void>
struct ReductionNoeud
{
	typedef NodeType TransformedNodeType;
	typedef IndicesType TransformedIndicesType;
};

template<typename Left, typename Op, typename Right, typename IndicesType, unsigned int IndexStart>
struct ReductionNoeud< Noeud<Left, Op, Right>, IndicesType, IndexStart,
		typename boost::enable_if<
				EstIdempotent<typename Left::ResultType, Op, typename Right::ResultType> 
				>::type>
{
	typedef Noeud<Left, Op, Right> BaseNode;
	//typedef Noeud<Left, void, void> TransformedNodeType;
	typedef BaseNode TransformedNodeType;

	//BOOST_MPL_ASSERT_RELATION( boost::mpl::size<IndicesType>::value, ==, 2);
	//BOOST_MPL_ASSERT_RELATION( IndexStart, ==, 0);

	static const unsigned int pos = IndexStart + 1;
	static const unsigned int end = IndexStart + 2;
	//typedef typename Erase<IndicesType, pos, end>::type TransformedIndicesType;
	//BOOST_MPL_ASSERT_RELATION( boost::mpl::size<IndicesType>::value, ==, boost::mpl::size<TransformedIndicesType>::value + 1);
	typedef IndicesType TransformedIndicesType;
};


// Réalise l'élimination d'un noeud binaire (deux opérandes idempotentes) si nécessaire
template<typename NodeType, typename IndicesType, unsigned int StartIndex, typename enabled = void>
struct VerificationReduction
{
	typedef NodeType TransformedNodeType;
	typedef IndicesType TransformedIndicesType;
};

// (A void void) Op (A void void) => A void void
template<typename Lhs, typename Op, typename Rhs, typename IndicesType, unsigned int StartIndex>
struct VerificationReduction< Noeud<Lhs, Op, Rhs>, IndicesType, StartIndex,
		typename boost::enable_if< boost::mpl::and_<
						EstIdempotent<typename Lhs::ResultType, Op, typename Rhs::ResultType>,
						boost::is_same<typename Lhs::OpType, void>,
						boost::is_same<typename Rhs::OpType, void>
						>
					>::type>
{
	typedef Noeud<Lhs, Op, Rhs> NodeType;
	typedef typename ReductionNoeud<NodeType, IndicesType, StartIndex>::TransformedNodeType TransformedNodeType;
	typedef typename ReductionNoeud<NodeType, IndicesType, StartIndex>::TransformedIndicesType TransformedIndicesType;
};


#endif
