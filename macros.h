#ifndef MACROS_H_INCLUDED
#define MACROS_H_INCLUDED

#include <boost/fusion/algorithm/iteration/accumulate.hpp>
#include <boost/fusion/include/accumulate.hpp>
#include <boost/version.hpp>


#define OPERANDS_POS_FROM_INDEX(ind_t,index) \
	boost::mpl::at_c<ind_t, index>::type::value

#define GET_OPERAND(realindex,args) \
	boost::fusion::at_c<realindex>(args)

#define DECLARE_COMMUTATIVE(type,med) \
	template <> struct EstCommutatif<type,med,type> : public boost::true_type {}

#define DECLARE_ASSOCIATIVE(type,med) \
	template <> struct EstAssociatif<type,med,type,med> : public boost::true_type {}

#define DECLARE_IDEMPOTENT(type,med) \
	template <> struct EstIdempotent<type,med,type> : public boost::true_type {}

#endif
