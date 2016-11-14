///////////////////////////////////////////////////////////////////////////////
//
// The MIT License
//
// Copyright (c) 2006 Division of Applied Mathematics, Brown University (USA),
// Department of Aeronautics, Imperial College London (UK), and Scientific
// Computing and Imaging Institute, University of Utah (USA).
//
// License for the specific language governing rights and limitations under
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////


#ifndef NOEUD_H_INCLUDED
#define NOEUD_H_INCLUDED


#define FUSION_MAX_VECTOR_SIZE 50
#include <boost/mpl/vector.hpp>
#include <boost/mpl/vector_c.hpp>
#include <boost/mpl/insert_range.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/sequence.hpp>
#include <boost/type_traits.hpp>
#include <boost/call_traits.hpp>


#include "traits.h"
#include "mediateur.h"


/////////////////////////////////////////////////////////////////////
// Séquence/Index d'opérandes jusqu'à n
/////////////////////////////////////////////////////////////////////

// cas général : construit une séquence en capsulant i) un entier encapsulé, ii) le reste de la séquence
template<typename T, int n>
struct InductiveSeq
{
	typedef typename InductiveSeq<T, n-1>::type ListType;
	typedef typename boost::mpl::push_back<ListType, boost::mpl::int_<n> >::type type; // type d'une séquence à n entiers
};

// séquence vide
template<typename T>
struct InductiveSeq<T, 0>
{
	typedef boost::mpl::vector_c<T, 0> type;
};


/////////////////////////////////////////////////////////////////////
// Patron principal pour représenter les composantes d'une expression
// cf. mediateur.h pour les opérateurs (médiateurs)
/////////////////////////////////////////////////////////////////////
template <typename Oper1, typename Op = void, typename Oper2 = void>
struct Noeud;

/////////////////////////////////////////////////////////////////////
// Feuille/ Noeud terminal
/////////////////////////////////////////////////////////////////////
template<typename T>
struct Noeud<T, void, void>
{
public:
	typedef Noeud<T, void, void> ThisType;	// alias vers le type encapsulé

	typedef typename EncapsData<T>::ResultType ResultType; 		// type canonique de T
	typedef typename EncapsData<T>::DataType DataType;	// type de retour (valeur ou réf)
	// Nb d'opérandes depuis ce noeud
	static const unsigned int TotalCount = 1;

	typedef boost::mpl::vector<DataType> MPLVectorType;
	typedef typename boost::fusion::result_of::as_vector<MPLVectorType>::type VectorType;	// construit un vecteur des types des opérandes
	typedef InductiveSeq<int, 1>::type Indices;					// vecteur des position des opérandes (type)
	typedef void OpType;
public:
	explicit Noeud(DataType value) : m_data(value) { }
	Noeud(const ThisType& rhs) : m_data(rhs.m_data) { }
	ThisType& operator=(const ThisType& rhs)
	{
		m_data = rhs.m_data;
		return *this;
	}
	const VectorType& GetData() const
	{
		return m_data;
	}
private:
	VectorType m_data;
};

/////////////////////////////////////////////////////////////////////
// Noeud Unaire (vers un noeud non-terminal)
/////////////////////////////////////////////////////////////////////
template<typename T1, typename TOp, typename T2, typename Op>
struct Noeud<Noeud<T1, TOp, T2>, Op, void>
{
public:
	typedef Op OpType;					// opérateur unaire
	typedef Noeud<T1, TOp, T2> ChildNodeType;		// sous-expr/unique opérande
	typedef Noeud<Noeud<T1, TOp, T2>, Op, void> ThisType;	// alias du type
	// nombre d'opérandes restants dans la sous-expression
	static const unsigned int TotalCount = ChildNodeType::TotalCount;

	typedef typename OpType::template ResultType<typename ChildNodeType::ResultType>::type ResultType;
	typedef typename ChildNodeType::Indices Indices;
	typedef typename ChildNodeType::VectorType VectorType;

	explicit Noeud(const ChildNodeType& value) : m_data(value.GetData()) { }
	Noeud(const ThisType& rhs) : m_data(rhs.m_data) { }
	ThisType& operator=(const ThisType& rhs) {
		m_data = rhs.m_data;
		return *this;
	}
	const VectorType& GetData() const
	{
		return m_data;
	}
private:
	VectorType m_data;
};

/////////////////////////////////////////////////////////////////////
// Noeud Binaire (vers deux noeuds non-terminaux)
/////////////////////////////////////////////////////////////////////
template<typename L1, typename LOp, typename L2, typename Op, typename R1, typename ROp, typename R2>
struct Noeud<Noeud<L1, LOp, L2>, Op, Noeud<R1, ROp, R2> >
{
public:
	typedef Noeud<L1, LOp, L2> Left;
	typedef Noeud<R1, ROp, R2> Right;
	typedef Op OpType;
	typedef Noeud<Left, Op, Right> ThisType;

	typedef typename Left::MPLVectorType LeftMPLVectorType;
	typedef typename boost::mpl::size<LeftMPLVectorType>::type LhsOperCount;

	typedef typename Right::MPLVectorType RightMPLVectorType;
	typedef typename boost::mpl::size<RightMPLVectorType>::type RhsOperCount;

	// nombre d'opérandes depuis ce noeud
	static const int TotalCount = LhsOperCount::value + RhsOperCount::value;

	typedef typename InductiveSeq<int, LhsOperCount::value + RhsOperCount::value - 1>::type Indices;

	typedef typename Left::VectorType LeftVectorType;        
	typedef typename Right::VectorType RightVectorType;

	typedef typename boost::mpl::insert_range
		     <
		         LeftMPLVectorType,
		         typename boost::mpl::end<LeftMPLVectorType>::type,
		         RightMPLVectorType
		     >::type MPLVectorType;
	// nouvelle séquence d'opérandes
	typedef typename boost::fusion::result_of::as_vector<MPLVectorType>::type VectorType;

	typedef typename OpType::template ResultType<typename Left::ResultType, typename Right::ResultType>::type ResultType;

public:
                 
    Noeud(const Left& lhs, const Right& rhs) : m_data(CreateMData(lhs, rhs)) { }
    Noeud(const ThisType& rhs) : m_data(rhs.m_data) { }
    ThisType& operator=(const ThisType& rhs)
    {
        m_data = rhs.m_data;
        return *this;
    }
    const VectorType& GetData() const { return m_data; }
    
private:
    // fusion (runtime) des listes d'opérandes
    VectorType CreateMData(const Left& lhs, const Right& rhs)
    {
        LeftVectorType l = lhs.GetData();
        RightVectorType r = rhs.GetData();
        boost::fusion::joint_view<LeftVectorType, RightVectorType> joint(l, r);
        return VectorType(joint);
    }                
    VectorType m_data;        
};


/////////////////////////////////////////////////////
/// Pour les sélecteurs statiques de méthodes selon 
/// le type de noeud à évaluer
/////////////////////////////////////////////////////
template<typename NodeType>
struct EstTerminal : public boost::false_type {};

template<typename DataType>
struct EstTerminal<Noeud<DataType, void, void> > : public boost::true_type {};

template<typename NodeType>
struct EstUnaire : public boost::false_type {};

template<typename ChildNodeType, typename UnaryType>
struct EstUnaire<Noeud<ChildNodeType, UnaryType, void> > : public boost::true_type {};

template<typename NodeType>
struct EstBinaire : public boost::true_type {}; // tous les cas différents de ceux du dessous

template<typename L1, typename OpType>
struct EstBinaire<Noeud<L1, OpType, void> > : public boost::false_type {};

template<typename L1>
struct EstBinaire<Noeud<L1, void, void> > : public boost::false_type {};



// juste un médiateur ici entre les partitions et les noeuds

template<typename L1, typename LOp, typename L2, typename RhsData>
Noeud<Noeud<L1, LOp, L2>, AddOp, Noeud<RhsData> >
operator+(const Noeud<L1, LOp, L2>& lhsNode, const RhsData& rhs)
{
	//std::cout << "N' << (A op B) + __" << std::endl;
	Noeud<RhsData> rhsNode(rhs);
	return Noeud<Noeud<L1, LOp, L2>, AddOp, Noeud<RhsData> >(lhsNode, rhsNode);
}

template<typename LhsData, typename R1, typename ROp, typename R2>
Noeud<Noeud<LhsData>, AddOp, Noeud<R1, ROp, R2> >
operator+(const LhsData& lhsData, const Noeud<R1, ROp, R2>& rhsNode)
{
	//std::cout << "N' << __ + (A op B)" << std::endl;
	Noeud<LhsData> lhsNode(lhsData);
	return Noeud<Noeud<LhsData>, AddOp, Noeud<R1, ROp, R2> >(lhsNode, rhsNode);
}
template<typename L1, typename LOp, typename L2, typename R1, typename ROp, typename R2>
Noeud<Noeud<L1, LOp, L2>, AddOp, Noeud<R1, ROp, R2> >
operator+(const Noeud<L1, LOp, L2>& lhsNode, const Noeud<R1, ROp, R2>& rhsNode)
{
	//std::cout << "N' << N(A op B) + N(C op D)" << std::endl;
	return Noeud<Noeud<L1, LOp, L2>, AddOp, Noeud<R1, ROp, R2> >(lhsNode, rhsNode);
}




template<typename L1, typename LOp, typename L2, typename RhsData>
Noeud<Noeud<L1, LOp, L2>, SubOp, Noeud<RhsData> >
operator-(const Noeud<L1, LOp, L2>& lhsNode, const RhsData& rhs)
{
	//std::cout << "N' << (A op B) - __" << std::endl;
	Noeud<RhsData> rhsNode(rhs);
	return Noeud<Noeud<L1, LOp, L2>, SubOp, Noeud<RhsData> >(lhsNode, rhsNode);
}

template<typename LhsData, typename R1, typename ROp, typename R2>
Noeud<Noeud<LhsData>, SubOp, Noeud<R1, ROp, R2> >
operator-(const LhsData& lhsData, const Noeud<R1, ROp, R2>& rhsNode)
{
	//std::cout << "N' << __ - (A op B)" << std::endl;
	Noeud<LhsData> lhsNode(lhsData);
	return Noeud<Noeud<LhsData>, SubOp, Noeud<R1, ROp, R2> >(lhsNode, rhsNode);
}
template<typename L1, typename LOp, typename L2, typename R1, typename ROp, typename R2>
Noeud<Noeud<L1, LOp, L2>, SubOp, Noeud<R1, ROp, R2> >
operator-(const Noeud<L1, LOp, L2>& lhsNode, const Noeud<R1, ROp, R2>& rhsNode)
{
	//std::cout << "N' << N(A op B) - N(C op D)" << std::endl;
	return Noeud<Noeud<L1, LOp, L2>, SubOp, Noeud<R1, ROp, R2> >(lhsNode, rhsNode);
}




template<typename L1, typename LOp, typename L2, typename RhsData>
Noeud<Noeud<L1, LOp, L2>, MulOp, Noeud<RhsData> >
operator*(const Noeud<L1, LOp, L2>& lhsNode, const RhsData& rhs)
{
	//std::cout << "N' << (A op B) * __" << std::endl;
	Noeud<RhsData> rhsNode(rhs);
	return Noeud<Noeud<L1, LOp, L2>, MulOp, Noeud<RhsData> >(lhsNode, rhsNode);
}

template<typename LhsData, typename R1, typename ROp, typename R2>
Noeud<Noeud<LhsData>, MulOp, Noeud<R1, ROp, R2> >
operator*(const LhsData& lhsData, const Noeud<R1, ROp, R2>& rhsNode)
{
	//std::cout << "N' << __ * (A op B)" << std::endl;
	Noeud<LhsData> lhsNode(lhsData);
	return Noeud<Noeud<LhsData>, MulOp, Noeud<R1, ROp, R2> >(lhsNode, rhsNode);
}
template<typename L1, typename LOp, typename L2, typename R1, typename ROp, typename R2>
Noeud<Noeud<L1, LOp, L2>, MulOp, Noeud<R1, ROp, R2> >
operator*(const Noeud<L1, LOp, L2>& lhsNode, const Noeud<R1, ROp, R2>& rhsNode)
{
	//std::cout << "N' << N(A op B) * N(C op D)" << std::endl;
	return Noeud<Noeud<L1, LOp, L2>, MulOp, Noeud<R1, ROp, R2> >(lhsNode, rhsNode);
}




//
template<typename Op, typename L, typename R>
Noeud<Noeud<L>, Op, Noeud<R> > CreateBinaryExpression(const L& lhs, const R& rhs)
{       
	//std::cout << "> Construction d'un noeud binaire de l'AST..." << std::endl;
	Noeud<L> lhs_node(lhs);
	Noeud<R> rhs_node(rhs);
	return Noeud<Noeud<L>, Op, Noeud<R> >(lhs_node, rhs_node);
}



#endif 







