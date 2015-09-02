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

#ifndef METAPHYSICL_DUALNUMBERVECTOR_H
#define METAPHYSICL_DUALNUMBERVECTOR_H


#include "metaphysicl/dualnumber_decl.h"
#include "metaphysicl/numbervector.h"
#include "metaphysicl/dualnumber.h"


namespace MetaPhysicL {

template <std::size_t size, typename T>
struct DerivativeType<NumberVector<size, T> >
{
  typedef NumberVector<size, typename DerivativeType<T>::type> type;
};


template <std::size_t size, typename T>
struct DerivativesType<NumberVector<size, T> >
{
  typedef NumberVector<size, typename DerivativesType<T>::type> type;
};


// DivergenceType of vectors is a scalar
template <std::size_t size, typename T>
struct DivergenceType<NumberVector<size, T> >
{
  typedef typename DerivativeType<T>::type type;
};


// DivergenceType of tensors sums over the last rank
template <std::size_t size1, std::size_t size2, typename T>
struct DivergenceType<NumberVector<size1, NumberVector<size2, T> > >
{
  typedef NumberVector
    <size1, typename DivergenceType<NumberVector<size2, T> >::type>
    type;
};



template <std::size_t size, typename T>
inline
typename DerivativeType<NumberVector<size, T> >::type
derivative(const NumberVector<size, T>& a, unsigned int derivativeindex)
{
  typename DerivativeType<NumberVector<size, T> >::type returnval;
  for (unsigned int i=0; i != size; ++i)
    returnval[i] = derivative(a[i], derivativeindex);

  return returnval;
}


template <std::size_t size, typename T>
inline
typename DerivativesType<NumberVector<size, T> >::type
derivatives(const NumberVector<size, T>& a)
{
  typename DerivativesType<NumberVector<size, T> >::type returnval;
  for (unsigned int i=0; i != size; ++i)
    returnval[i] = derivatives(a[i]);

  return returnval;
}


template <std::size_t size, typename T, unsigned int derivativeindex>
struct DerivativeOf<NumberVector<size, T>, derivativeindex>
{
  static
  typename DerivativeType<NumberVector<size, T> >::type
  derivative(const NumberVector<size, T>& a)
  {
    typename DerivativeType<NumberVector<size, T> >::type returnval;
    for (unsigned int i=0; i != size; ++i)
      returnval[i] = DerivativeOf<T,derivativeindex>::derivative(a[i]);
  
    return returnval;
  }
};


// For a vector of values a[i] each of which is a scalar with a
// defined derivative, the divergence is the sum of
// derivative_wrt_xi(a[i])
template <std::size_t size, typename T>
inline
typename boostcopy::enable_if_c<
  ScalarTraits<T>::value,
  typename DivergenceType<NumberVector<size, T> >::type>::type
divergence(const NumberVector<size, T>& a)
{
  typename DivergenceType<NumberVector<size, T> >::type returnval = 0;

  for (unsigned int i=0; i != size; ++i)
    returnval += derivative(a[i], i);

  return returnval;
}

// For a tensor of values, we take the divergence with respect to the
// last index.
template <std::size_t size, typename T>
inline
typename boostcopy::enable_if_c<
  !ScalarTraits<T>::value,
  typename DivergenceType<NumberVector<size, T> >::type>::type
divergence(const NumberVector<size, T>& a)
{
  typename DivergenceType<NumberVector<size, T> >::type returnval = 0;

  for (unsigned int i=0; i != size; ++i)
    returnval[i] = divergence(a[i]);

  return returnval;
}


// For a vector of values, the gradient is going to be a tensor
template <std::size_t size, typename T>
inline
NumberVector<size, typename T::derivatives_type>
gradient(const NumberVector<size, T>& a)
{
  NumberVector<size, typename T::derivatives_type> returnval;

  for (unsigned int i=0; i != size; ++i)
    returnval[i] = gradient(a[i]);

  return returnval;
}

// DualNumber is subordinate to NumberVector

#define DualNumberVector_comparisons(templatename) \
template<typename T, typename D, std::size_t size, typename T2, bool reverseorder> \
struct templatename<NumberVector<size, T2>, DualNumber<T, D>, reverseorder> { \
  typedef NumberVector<size, typename Symmetric##templatename<DualNumber<T, D>, T2, reverseorder>::supertype> supertype; \
}

DualNumberVector_comparisons(CompareTypes);
DualNumberVector_comparisons(PlusType);
DualNumberVector_comparisons(MinusType);
DualNumberVector_comparisons(MultipliesType);
DualNumberVector_comparisons(DividesType);
DualNumberVector_comparisons(AndType);
DualNumberVector_comparisons(OrType);

} // namespace MetaPhysicL

#endif // METAPHYSICL_DUALNUMBERVECTOR_H
