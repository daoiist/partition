#ifndef TRAITS_H_INCLUDED
#define TRAITS_H_INCLUDED


#include "mediateur.h"


// pour les types avec constructeurs
template<typename T, typename enabled = void>
struct EncapsData
{
	// caractérise le type en retirant tous qualificateurs (const/réf)
	typedef typename boost::remove_cv<typename boost::remove_reference<T>::type>::type ResultType;

	// référence constante du type
	typedef typename boost::call_traits<T>::const_reference DataType;
};


// pour les types primitifs
template<typename T>
struct EncapsData<T, 
		typename boost::enable_if< boost::is_arithmetic<T> >::type
		>
{
	// caractérise le type en retirant tous qualificateurs (const/réf)
	typedef typename boost::remove_cv<typename boost::remove_reference<T>::type>::type ResultType;
	     
	// si le noeud est entier/flottant, passage par valeur plutôt que par référence
	typedef typename boost::call_traits<T>::value_type DataType;
};


/////////////////////////////////////////////////////////////////////////

template<typename L, typename Op, typename R, typename enabled = void>
struct EstCommutatif : public boost::false_type {};

template<typename R, typename T>
struct EstCommutatif<R, AddOp, T> : public boost::true_type {};
template<typename R, typename T>
struct EstCommutatif<R, MulOp, T> : public boost::true_type {};

/////////////////////////////////////////////////////////////////////////

template<typename T1, typename Op1, typename T2, typename Op2>
struct EstAssociatif : public boost::false_type {};

template<typename T>
struct EstAssociatif<T, AddOp, T, AddOp> : public boost::true_type {};
template<typename T>
struct EstAssociatif<T, MulOp, T, MulOp> : public boost::true_type {};

/////////////////////////////////////////////////////////////////////////

template<typename T1, typename Op1, typename T2,typename enabled = void>
struct EstIdempotent : public boost::false_type {};




#endif
