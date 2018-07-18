//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
// 
// MetaPhysicL - A metaprogramming library for physics calculations
//
// Copyright (C) 2013 The PECOS Development Team
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the Version 2.1 GNU Lesser General
// Public License as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc. 51 Franklin Street, Fifth Floor,
// Boston, MA  02110-1301  USA
//
//-----------------------------------------------------------------------el-
//
// $Id: core.h 37197 2013-02-21 05:49:09Z roystgnr $
//
//--------------------------------------------------------------------------

#ifndef METAPHYSICL_DUALNUMBERARRAY_H
#define METAPHYSICL_DUALNUMBERARRAY_H


#include "metaphysicl/dualnumber.h"
#include "metaphysicl/numberarray.h"


namespace MetaPhysicL {

template <std::size_t size, typename T>
struct DerivativeType<NumberArray<size, T> >
{
  typedef NumberArray<size, typename DerivativeType<T>::type> type;
};


template <std::size_t size, typename T>
struct DerivativesType<NumberArray<size, T> >
{
  typedef NumberArray<size, typename DerivativesType<T>::type> type;
};


template <std::size_t size, typename T>
struct DivergenceType<NumberArray<size, T> >
{
  typedef NumberArray<size, typename DivergenceType<T>::type> type;
};


template <std::size_t size, typename T>
inline
typename DerivativeType<NumberArray<size, T> >::type
derivative(const NumberArray<size, T>& a, unsigned int derivativeindex)
{
  typename DerivativeType<NumberArray<size, T> >::type returnval;
  for (unsigned int i=0; i != size; ++i)
    returnval[i] = derivative(a[i], derivativeindex);

  return returnval;
}

template <std::size_t size, typename T>
inline
typename DerivativesType<NumberArray<size, T> >::type
derivatives(const NumberArray<size, T>& a)
{
  typename DerivativesType<NumberArray<size, T> >::type returnval;
  for (unsigned int i=0; i != size; ++i)
    returnval[i] = derivatives(a[i]);

  return returnval;
}


template <std::size_t size, typename T, unsigned int derivativeindex>
struct DerivativeOf<NumberArray<size, T>, derivativeindex>
{
  static
  typename DerivativeType<NumberArray<size, T> >::type
  derivative(const NumberArray<size, T>& a)
  {
    typename DerivativeType<NumberArray<size, T> >::type returnval;
    for (unsigned int i=0; i != size; ++i)
      returnval[i] = DerivativeOf<T,derivativeindex>::derivative(a[i]);
  
    return returnval;
  }
};


// For an array of values a[i] each of which has a defined divergence,
// the divergence is the array of divergence(a[i])

template <std::size_t size, typename T>
inline
typename DivergenceType<NumberArray<size, T> >::type
divergence(const NumberArray<size, T>& a)
{
  typename DivergenceType<NumberArray<size, T> >::type
    returnval = 0;

  for (unsigned int i=0; i != size; ++i)
    returnval[i] = divergence(a[i]);

  return returnval;
}


// For a vector of values, the gradient is going to be a vector of gradients
template <std::size_t size, typename T>
inline
NumberArray<size, typename DerivativeType<T>::type >
gradient(const NumberArray<size, T>& a)
{
  NumberArray<size, typename DerivativeType<T>::type> returnval;

  for (unsigned int i=0; i != size; ++i)
    returnval[i] = gradient(a[i]);

  return returnval;
}

// DualNumber is subordinate to NumberArray

#define DualNumberArray_comparisons(templatename) \
template<typename T, typename D, std::size_t size, typename T2, bool reverseorder> \
struct templatename<NumberArray<size, T2>, DualNumber<T, D>, reverseorder> { \
  typedef NumberArray<size, typename Symmetric##templatename<DualNumber<T, D>, T2, reverseorder>::supertype> supertype; \
}

DualNumberArray_comparisons(CompareTypes);
DualNumberArray_comparisons(PlusType);
DualNumberArray_comparisons(MinusType);
DualNumberArray_comparisons(MultipliesType);
DualNumberArray_comparisons(DividesType);
DualNumberArray_comparisons(AndType);
DualNumberArray_comparisons(OrType);

} // namespace MetaPhysicL

#endif // METAPHYSICL_DUALNUMBERARRAY_H
