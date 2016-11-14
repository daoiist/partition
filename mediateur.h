
#ifndef MEDIATEUR_H_INCLUDED
#define MEDIATEUR_H_INCLUDED

#include <boost/typeof/typeof.hpp>

struct AddOp 
{
	template<typename L, typename R>
	struct ResultType
	{
		BOOST_TYPEOF_NESTED_TYPEDEF_TPL(nested, Add(L(), R()));
		typedef typename nested::type type;
	};

	template<typename L, typename R>
	static void OpEqual(L& accum, const R& rhs)
	{
		//std::cout << "OpEq " << std::endl;
		AddEqual(accum, rhs);
	}

	template<typename L, typename R>
	static void Op(typename ResultType<L, R>::type& accumulator, const L& lhs, const R& rhs)
	{
		//std::cout << "Op " << std::endl;
		return Add(accumulator, lhs, rhs);
	}
};




struct SubOp 
{
	template<typename L, typename R>
	struct ResultType
	{
		BOOST_TYPEOF_NESTED_TYPEDEF_TPL(nested, Sub(L(), R()));
		typedef typename nested::type type;
	};

	template<typename L, typename R>
	static void OpEqual(L& accum, const R& rhs)
	{
		SubEqual(accum, rhs);
	}

	template<typename L, typename R>
	static void Op(typename ResultType<L, R>::type& accumulator, const L& lhs, const R& rhs)
	{
		return Sub(accumulator, lhs, rhs);
	}
};




struct MulOp 
{
	template<typename L, typename R>
	struct ResultType
	{
		BOOST_TYPEOF_NESTED_TYPEDEF_TPL(nested, Mul(L(), R()));
		typedef typename nested::type type;
	};

	template<typename L, typename R>
	static void OpEqual(L& accum, const R& rhs)
	{
		//std::cout << "OpEq " << std::endl;
		MulEqual(accum, rhs);
	}

	template<typename L, typename R>
	static void Op(typename ResultType<L, R>::type& accumulator, const L& lhs, const R& rhs)
	{
		//std::cout << "Op " << std::endl;
		return Mul(accumulator, lhs, rhs);
	}
};



#endif
