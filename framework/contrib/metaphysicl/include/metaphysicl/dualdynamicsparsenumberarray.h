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

#ifndef METAPHYSICL_DUALDYNAMICSPARSENUMBERARRAY_H
#define METAPHYSICL_DUALDYNAMICSPARSENUMBERARRAY_H


#include "metaphysicl/dualdynamicsparsenumberarray_decl.h"

#include "metaphysicl/dualnumber.h"
#include "metaphysicl/dynamicsparsenumberarray.h"


namespace MetaPhysicL {

template <typename T, typename I>
inline
typename DerivativeType<DynamicSparseNumberArray<T, I> >::type
derivative (const DynamicSparseNumberArray<T, I>& a,
            unsigned int derivativeindex)
{
  std::size_t index_size = a.size();

  typename DerivativeType<DynamicSparseNumberArray<T, I> >::type returnval;
  returnval.nude_indices() = a.nude_indices();
  returnval.resize(index_size);

  for (unsigned int i=0; i != index_size; ++i)
    returnval.raw_at(i) = derivative(a.raw_at(i),derivativeindex);
  return returnval;
}


template <typename T, typename I>
inline
typename DerivativesType<DynamicSparseNumberArray<T, I> >::type
derivatives (const DynamicSparseNumberArray<T, I>& a)
{
  std::size_t index_size = a.size();

  typename DerivativesType<DynamicSparseNumberArray<T, I> >::type returnval;

  returnval.nude_indices() = a.nude_indices();
  returnval.resize(index_size);

  for (unsigned int i=0; i != index_size; ++i)
    returnval.raw_at(i) = derivatives(a.raw_at(i));
  return returnval;
}


template <typename T, typename I, unsigned int derivativeindex>
typename DerivativeType<DynamicSparseNumberArray<T, I> >::type
DerivativeOf<DynamicSparseNumberArray<T, I>, derivativeindex>::derivative (const DynamicSparseNumberArray<T, I>& a)
{
  std::size_t index_size = a.size();

  typename DerivativeType<DynamicSparseNumberArray<T, I> >::type returnval;

  returnval.nude_indices() = a.nude_indices();
  returnval.resize(index_size);

  for (unsigned int i=0; i != index_size; ++i)
    returnval.raw_at(i) = DerivativeOf<T,derivativeindex>::derivative(a.raw_at(i));
  return returnval;
}



// For a vector of values a[i] each of which has a defined gradient,
// the divergence is the sum of derivative_wrt_xi(a[i])

// For a tensor of values, we take the divergence with respect to the
// first index.
template <typename T, typename I>
inline
typename DerivativeType<T>::type
divergence(const DynamicSparseNumberArray<T, I>& a)
{
  typename DerivativeType<T>::type returnval = 0;

  // FIXME
  metaphysicl_not_implemented();

  return returnval;
}


// For a vector of values, the gradient is going to be a tensor
template <typename T, typename I>
inline
DynamicSparseNumberArray<typename T::derivatives_type, I>
gradient(const DynamicSparseNumberArray<T, I>& a)
{
  static const unsigned int index_size = a.size();

  DynamicSparseNumberArray<typename T::derivatives_type, I> returnval;

  returnval.nude_indices() = a.nude_indices();
  returnval.resize(index_size);

  for (unsigned int i=0; i != index_size; ++i)
    returnval.raw_at(i) = gradient(a.raw_at(i));

  return returnval;
}

} // namespace MetaPhysicL

#endif // METAPHYSICL_DUALDYNAMICSPARSENUMBERARRAY_H
