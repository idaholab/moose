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

#ifndef METAPHYSICL_DUALDYNAMICSPARSENUMBERARRAY_DECL_H
#define METAPHYSICL_DUALDYNAMICSPARSENUMBERARRAY_DECL_H


#include "metaphysicl/dualnumber_decl.h"
#include "metaphysicl/dynamicsparsenumberarray_decl.h"


namespace MetaPhysicL {

template <typename T, typename I>
struct DerivativeType<DynamicSparseNumberArray<T, I> >
{
  typedef DynamicSparseNumberArray<typename DerivativeType<T>::type, I> type;
};


template <typename T, typename I>
struct DerivativesType<DynamicSparseNumberArray<T, I> >
{
  typedef DynamicSparseNumberArray<typename DerivativesType<T>::type, I> type;
};


template <typename T, typename I>
inline
typename DerivativeType<DynamicSparseNumberArray<T, I> >::type
derivative (const DynamicSparseNumberArray<T, I>& a,
            unsigned int derivativeindex);


template <typename T, typename I>
inline
typename DerivativesType<DynamicSparseNumberArray<T, I> >::type
derivatives (const DynamicSparseNumberArray<T, I>& a);


template <typename T, typename I, unsigned int derivativeindex>
struct DerivativeOf<DynamicSparseNumberArray<T, I>, derivativeindex>
{
  static
  typename DerivativeType<DynamicSparseNumberArray<T, I> >::type
  derivative (const DynamicSparseNumberArray<T, I>& a);
};



// For a vector of values a[i] each of which has a defined gradient,
// the divergence is the sum of derivative_wrt_xi(a[i])

// For a tensor of values, we take the divergence with respect to the
// first index.
template <typename T, typename I>
inline
typename DerivativeType<T>::type
divergence(const DynamicSparseNumberArray<T, I>& a);


// For a vector of values, the gradient is going to be a tensor
template <typename T, typename I>
inline
DynamicSparseNumberArray<typename T::derivatives_type, I>
gradient(const DynamicSparseNumberArray<T, I>& a);

// DualNumber is subordinate to DynamicSparseNumberArray

#define DualDynamicSparseNumberArray_comparisons(templatename) \
template<typename T, typename T2, typename D, typename I, bool reverseorder> \
struct templatename<DynamicSparseNumberArray<T2, I>, DualNumber<T, D>, reverseorder> { \
  typedef DynamicSparseNumberArray<typename Symmetric##templatename<T2,DualNumber<T, D>,reverseorder>::supertype, I> supertype; \
}

DualDynamicSparseNumberArray_comparisons(CompareTypes);
DualDynamicSparseNumberArray_comparisons(PlusType);
DualDynamicSparseNumberArray_comparisons(MinusType);
DualDynamicSparseNumberArray_comparisons(MultipliesType);
DualDynamicSparseNumberArray_comparisons(DividesType);
DualDynamicSparseNumberArray_comparisons(AndType);
DualDynamicSparseNumberArray_comparisons(OrType);

} // namespace MetaPhysicL

#endif // METAPHYSICL_DUALDYNAMICSPARSENUMBERARRAY_DECL_H
