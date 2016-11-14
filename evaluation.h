#ifndef EVALUATION_H_INCLUDED
#define EVALUATION_H_INCLUDED

#include "macros.h"
#include "noeud.h"
#include "optimisation.h"

#include <boost/fusion/algorithm/iteration/accumulate.hpp>
#include <boost/fusion/include/accumulate.hpp>
#include <boost/version.hpp>


template<typename DataType, typename NodeType, typename Indices, unsigned int StartIndex>
struct GenerateTemporaryResult
{
	template<typename ArgVectorType>
	static void Apply(const ArgVectorType& tree);
};


/////////////////////////////////////////////////////////
/// Patron des foncteurs d'évaluation des noeuds d'un AST
/// 3 spécialisations selon l'arité du noeud (0,1 ou 2)
/////////////////////////////////////////////////////////
template<typename NodeType, typename IndicesType, unsigned int index=0>
struct EvaluationRecursive;

//////////////////////////////////////////////
// Arité 0
//////////////////////////////////////////////

template<typename Type, typename IndicesType, unsigned int index>
struct EvaluationRecursive<Noeud<Type, void, void>, IndicesType, index>
{
	static const unsigned int MappedIndex = OPERANDS_POS_FROM_INDEX(IndicesType, index);

	template<typename ResultType, typename ArgumentVectorType>
	static void Evaluate(ResultType& accumulator, const ArgumentVectorType& args)
	{
		accumulator = GET_OPERAND(MappedIndex, args);
	}
};

//////////////////////////////////////////////
// Arité 1
//////////////////////////////////////////////

template<typename ChildType, typename OpType, typename IndicesType, unsigned int index>
struct EvaluationRecursive<Noeud<ChildType, OpType, void>, IndicesType, index>
{
	template<typename ResultType, typename ArgumentVectorType>
	static void Evaluate(ResultType& accumulator, const ArgumentVectorType& args)
	{
		EvaluationRecursive<ChildType, IndicesType, index>::Evaluate(accumulator, args);
		OpType::Op(accumulator);
	}
};

//////////////////////////////////////////////////////////////////
// Stratégie d'évaluation d'un noeud binaire
//////////////////////////////////////////////////////////////////

// Durant l'évaluation en BFS, l'analyse du SAD nécessite la création
// d'un accumulateur temporaire si et seulement si il contient une expression
template<typename NodeType, typename IndicesType, unsigned int index>
struct EvaluateRightST;

// Cas du noeud constant 
template<typename Type, typename IndicesType, unsigned int index>
struct EvaluateRightST< Noeud<Type, void, void>, IndicesType, index>
{
	static const unsigned int MappedIndex = OPERANDS_POS_FROM_INDEX(IndicesType,index);

	typedef const Type& ResultType;
	template<typename ArgumentVectorType>
	static ResultType Evaluate(const ArgumentVectorType& args)
	{
		return GET_OPERAND(MappedIndex, args);
	}
};

// Cas d'une sous-expression : le résultat de l'évaluation du sous-arbre droit est stockée
// dans une variable temporaire
template<typename Arg1, typename Op, typename Arg2, typename IndicesType, unsigned int index>
struct EvaluateRightST<Noeud<Arg1, Op, Arg2>, IndicesType, index>
{
	typedef Noeud<Arg1, Op, Arg2> NodeType;
	typedef typename NodeType::ResultType ResultType;

	static const unsigned int LMI = OPERANDS_POS_FROM_INDEX(IndicesType,index), RMI = OPERANDS_POS_FROM_INDEX(IndicesType,index + 1);

	template<typename ArgumentVectorType>
	static ResultType Evaluate(const ArgumentVectorType& args)
	{
		ResultType temp = GenerateTemporaryResult<ResultType, NodeType, IndicesType, index>::Apply(args);
		EvaluationRecursive<NodeType, IndicesType, index>::Evaluate(temp, args);
		return temp;
	}
};

// Cas général : évaluation séparée du SAG et du SAD : celui-ci nécessite une variable temporaire
template<typename LhsType, typename Op, typename RhsType, typename IndicesType, unsigned int index>
struct EvaluateBinaryNode
	{
	static const int rhsNodeIndex = index + LhsType::TotalCount;
	typedef typename LhsType::ResultType LhsResultType;
	
	typedef EvaluateRightST<RhsType, IndicesType, rhsNodeIndex> EvaluateRightSTType;
	typedef typename EvaluateRightSTType::ResultType RhsTempType;    

	// On vérifie que le type de retour des deux sous-expressions est compatible
	template<typename ResultType, typename ArgumentVectorType>
	static void Evaluate(ResultType& accumulator, const ArgumentVectorType& args/*, typename boost::enable_if
                <boost::is_same<ResultType, LhsResultType> >::type* dummy = 0*/)
	{
		//std::cout << accumulator.name() << std::endl;
		EvaluationRecursive<LhsType, IndicesType, index>::Evaluate(accumulator, args);
		RhsTempType rhs = EvaluateRightSTType::Evaluate(args);
		//std::cout << accumulator.name() << " " << rhs.name()  << std::endl;
		// appel au médiateur et évaluation finale
		Op::OpEqual(accumulator, rhs);
	}
	/* template<typename ResultType, typename ArgumentVectorType>
            static void Evaluate(ResultType& accumulator, const ArgumentVectorType& args, 
                typename boost::enable_if
                <boost::mpl::not_<boost::is_same<ResultType, LhsResultType>> >::type* dummy = 0)
            {
                LhsResultType temp = GenerateTemporaryResult<LhsResultType, LhsType, IndicesType, index>::Apply(args);
                EvaluationRecursive<LhsType, IndicesType, index>::Evaluate(temp, args);
                RhsTempType rhs = EvaluateRightSTType::Evaluate(args);
                Op::Op(accumulator, temp, rhs);
            }*/
};

// Cas terminale : les deux SA's sont des instances
template<typename L, typename Op, typename R, typename IndicesType, unsigned int index>
struct EvaluateBinaryNode<Noeud<L, void, void>, Op, Noeud<R, void, void>, IndicesType, index>
{
	static const unsigned int LMI = OPERANDS_POS_FROM_INDEX(IndicesType,index), RMI = OPERANDS_POS_FROM_INDEX(IndicesType,index + 1);

	template<typename ResultType, typename ArgumentVectorType>
	static void Evaluate(ResultType& accumulator, const ArgumentVectorType& args)
	{
		// appel au médiateur et évaluation finale
		Op::Op(accumulator, GET_OPERAND(LMI, args), GET_OPERAND(RMI, args));
	}
};
////////////////////////////////////////////////////
// Arité 2
// Point d'entrée de toutes les évaluations d'AST
// (non triviaux) post-optimisations de sa structure
////////////////////////////////////////////////////
template<typename LhsType, typename OpType, typename RhsType, typename IndicesType, unsigned int index>
struct EvaluationRecursive<Noeud<LhsType, OpType, RhsType>, IndicesType, index>
{
	template<typename ResultType, typename ArgumentVectorType>
	static void Evaluate(ResultType& accumulator, const ArgumentVectorType& args)
	{
		EvaluateBinaryNode<LhsType, OpType, RhsType, IndicesType, index>::Evaluate(accumulator, args);
	}
};

    // During evaluation, we need to detect scenarios such as 
    // A = A*B + A
    //
    // If we evaluate A*B first, then the accumulator value will be overwritten and the 
    // expression will be incorrect.  When an alias is detected, the expression evaluator
    // creates a temporary for the accumulator.
    //
    // Users can specialize this if the default behaviors are not adequate.
    template<typename T, typename R>
    struct IsAlias
    {
        static bool Apply(const T& lhs, const R& rhs)
        {
            return false;
        }
    };

    template<typename T>
    struct IsAlias<T, T>
    {
        static bool Apply(const T& lhs, const T& rhs)
        {
            return &lhs == &rhs;
        }
    };

    struct ExpressionEvaluator
    {
        /// \brief A class that is used with boost::fusion::accumulate to iterate an expression's
        /// argument list to determine if there are any aliases.
        template<typename T>
        struct ContainsAliasAccumulator
        {
            ContainsAliasAccumulator(const T& v) :
                value(v)
            {
            }

            // The ordering of the parameters passed to this class in boost::fusion::accumulate
            // changed in boost 1.42
#if BOOST_VERSION < 104200
            template<typename ElementType>
            unsigned int operator()(const ElementType& rhs, const unsigned int& accum) const
#else
            template<typename ElementType>
            unsigned int operator()(const unsigned int& accum, const ElementType& rhs) const
#endif
            {
                return accum + IsAlias<T, ElementType>::Apply(value, rhs);
            }

            typedef int result_type;
            typedef int result;

            const T& value;
        };

        /// \brief Determines if the accumulator is aliased anywhere in the expression.
        template<typename Expression>
        static bool ContainsAlias(const Expression& expression, typename Expression::ResultType& accum)
        {
            ContainsAliasAccumulator<typename Expression::ResultType> containsAliasAccumulator(accum);
            int numAliases = boost::fusion::accumulate(expression.GetData(), 0, containsAliasAccumulator);
            return numAliases > 0;
        }

/// \brief Evaluates an expression, creating a new object as the accumulator.
///
/// It is not always possible to modify the class definitions of the objects for which 
/// expression templates will be used.  In those cases, user code can call this evaluate 
/// method to evaluate the expression.
template<typename Expression>
static typename Expression::ResultType Evaluate(const Expression& expression)
{
	// Perform the optimizations on the parse three.
	typedef typename GlobalOptimisation<Expression>::TransformedNodeType OptimizedParseTree;
	typedef typename GlobalOptimisation<Expression>::TransformedIndicesType TransformedIndicesType;
	typedef typename Expression::ResultType ResultType;

	//std::cout << " => Eval #Opt1 " << std::endl;
	ResultType result = GenerateTemporaryResult<ResultType, OptimizedParseTree, TransformedIndicesType, 0>::Apply(expression.GetData());
	EvaluationRecursive<OptimizedParseTree, TransformedIndicesType>::Evaluate(result, expression.GetData());
	return result;
}

/// \brief This method evaluates the expression, storing the results in accum.
///
/// It is expected that this method is called from within a class constructor or assignment operator, 
/// with accum = *this.
template<typename Expression>
static void Evaluate(const Expression& expression, typename Expression::ResultType& accum)
{
	// Stats
	//static const unsigned int tmp_count = SADCount<Expression>::Value;
	// Perform the optimizations on the parse three.
	typedef typename GlobalOptimisation<Expression>::TransformedNodeType OptimizedParseTree;
	typedef typename GlobalOptimisation<Expression>::TransformedIndicesType TransformedIndicesType;
	typedef typename Expression::ResultType ResultType;
	//static const unsigned int tmp_count_opt = SADCount<OptimizedParseTree>::Value;

	//std::cout << " Optimisation : " << tmp_count << " => " << tmp_count_opt << std::endl;
	if( ContainsAlias(expression, accum) )
	{
		//std::cout << " Alias" << std::endl;
		ResultType temp = GenerateTemporaryResult<ResultType, OptimizedParseTree, TransformedIndicesType, 0>::Apply(expression.GetData());
		EvaluationRecursive<OptimizedParseTree, TransformedIndicesType>::Evaluate(temp, expression.GetData());
		accum = temp;
	}
	else
		EvaluationRecursive<OptimizedParseTree, TransformedIndicesType>::Evaluate(accum, expression.GetData());
}

        template<typename Expression>
        static void EvaluateWithoutAliasingCheck(const Expression& expression, typename Expression::ResultType& accum)
        {
		//std::cout << " => Eval #Opt3 " << std::endl;
            // Perform the optimizations on the parse three.
            //typedef typename RemoveUnecessaryTemporaries<Expression>::TransformedNodeType OptimizedParseTree;
            //typedef typename RemoveUnecessaryTemporaries<Expression>::TransformedIndicesType TransformedIndicesType;

            EvaluationRecursive<typename Expression::ThisType,typename Expression::Indices>::Evaluate(accum, expression.GetData());
        }
    };


#endif //EXPRESSION_TEMPLATES_EXPRESSION_EVALUATOR_HPP

