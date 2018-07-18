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


#ifndef METAPHYSICL_DYNAMICSPARSENUMBERARRAY_H
#define METAPHYSICL_DYNAMICSPARSENUMBERARRAY_H

#include "metaphysicl/dynamicsparsenumberarray_decl.h"
#include "metaphysicl/dynamicsparsenumberbase.h"

namespace MetaPhysicL {

template <typename T, typename I>
inline
DynamicSparseNumberArray<T,I>::DynamicSparseNumberArray() {}

template <typename T, typename I>
inline
DynamicSparseNumberArray<T,I>::DynamicSparseNumberArray(const T& val) {
  // Avoid unused variable warnings in opt mode.
  (void)val;
  // This makes no sense unless val is 0!
#ifndef NDEBUG
  if (val)
    throw std::domain_error("Cannot initialize DynamicSparseNumberArray with non-zero scalar");
#endif
}

template <typename T, typename I>
template <typename T2>
inline
DynamicSparseNumberArray<T,I>::DynamicSparseNumberArray(const T2& val) {
  // Avoid unused variable warnings in opt mode.
  (void)val;
  // This makes no sense unless val is 0!
#ifndef NDEBUG
  if (val)
    throw std::domain_error("Cannot initialize DynamicSparseNumberArray with non-zero scalar");
#endif
}

template <typename T, typename I>
template <typename T2, typename I2>
inline
DynamicSparseNumberArray<T,I>::DynamicSparseNumberArray(DynamicSparseNumberArray<T2, I2> src) :
  DynamicSparseNumberBase<T,I,MetaPhysicL::DynamicSparseNumberArray>(src) {}


template <typename T, typename I>
template <typename T2, typename I2>
inline
DynamicSparseNumberArray
  <typename DotType<T,T2>::supertype,
   typename CompareTypes<I, I2>::supertype>
DynamicSparseNumberArray<T,I>::dot (const DynamicSparseNumberArray<T2,I2>& /*a*/) const
{
  typedef typename DotType<T,T2>::supertype TS;
  typedef typename CompareTypes<I, I2>::supertype IS;

  DynamicSparseNumberArray<TS, IS> returnval;

  // FIXME
  metaphysicl_not_implemented();

  return returnval;
}

template <typename T, typename I>
template <typename T2, typename I2>
inline
DynamicSparseNumberArray<
  typename OuterProductType<T,T2>::supertype,
  typename CompareTypes<I, I2>::supertype>
DynamicSparseNumberArray<T,I>::outerproduct (const DynamicSparseNumberArray<T2, I2>& /*a*/) const
{
  typedef typename OuterProductType<T,T2>::supertype TS;
  typedef typename CompareTypes<I, I2>::supertype IS;
  DynamicSparseNumberArray<TS, IS> returnval;

  // FIXME
  metaphysicl_not_implemented();

  return returnval;
}


//
// Non-member functions
//

template <typename T, typename I, typename I2>
inline
DynamicSparseNumberArray<DynamicSparseNumberArray<T, I>, I2>
transpose(const DynamicSparseNumberArray<DynamicSparseNumberArray<T, I2>, I>& /*a*/)
{
  DynamicSparseNumberArray<DynamicSparseNumberArray<T, I>, I2> returnval;

  metaphysicl_not_implemented();

  return returnval;
}


template <typename T, typename I>
inline
DynamicSparseNumberArray<typename SumType<T>::supertype, I>
sum (const DynamicSparseNumberArray<T, I> &a)
{
  std::size_t index_size = a.size();

  DynamicSparseNumberArray<typename SumType<T>::supertype, I>
    returnval;
  returnval.resize(index_size);

  for (unsigned int i=0; i != index_size; ++i) {
    returnval.raw_at(i) = a.raw_at(i).sum();
    returnval.raw_index(i) = a.raw_index(i);
  }

  return returnval;
}



DynamicSparseNumberBase_op(DynamicSparseNumberArray, +, Plus)       // Union)
DynamicSparseNumberBase_op(DynamicSparseNumberArray, -, Minus)      // Union)
DynamicSparseNumberBase_op(DynamicSparseNumberArray, *, Multiplies) // Intersection)
DynamicSparseNumberBase_op(DynamicSparseNumberArray, /, Divides)    // First)


template <typename T, typename I>
inline
typename RawType<DynamicSparseNumberArray<T, I> >::value_type
RawType<DynamicSparseNumberArray<T, I> >::value(const DynamicSparseNumberArray<T, I>& a)
{
  value_type returnval;
  returnval.nude_indices() = a.nude_indices();

  std::size_t index_size = a.size();
  returnval.nude_data().resize(index_size);

  for (unsigned int i=0; i != index_size; ++i)
    returnval.raw_at(i) = RawType<T>::value(a.raw_at(i));
  return returnval;
}

} // namespace MetaPhysicL

#endif // METAPHYSICL_DYNAMICSPARSENUMBERARRAY_H
