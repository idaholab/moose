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


#ifndef METAPHYSICL_DYNAMICSPARSENUMBERVECTOR_H
#define METAPHYSICL_DYNAMICSPARSENUMBERVECTOR_H

#include "metaphysicl/dynamicsparsenumbervector_decl.h"
#include "metaphysicl/dynamicsparsenumberbase.h"

namespace MetaPhysicL {

template <typename T, typename I>
inline
DynamicSparseNumberVector<T,I>::DynamicSparseNumberVector() {}

template <typename T, typename I>
inline
DynamicSparseNumberVector<T,I>::DynamicSparseNumberVector(const T& val) {
  // This makes no sense unless val is 0!
#ifndef NDEBUG
  if (val)
    throw std::domain_error("Cannot initialize DynamicSparseNumberVector with non-zero scalar");
#endif
}

template <typename T, typename I>
template <typename T2>
inline
DynamicSparseNumberVector<T,I>::DynamicSparseNumberVector(const T2& val) {
  // This makes no sense unless val is 0!
#ifndef NDEBUG
  if (val)
    throw std::domain_error("Cannot initialize DynamicSparseNumberVector with non-zero scalar");
#endif
}

template <typename T, typename I>
template <typename T2, typename I2>
inline
DynamicSparseNumberVector<T,I>::DynamicSparseNumberVector(DynamicSparseNumberVector<T2, I2> src) :
  DynamicSparseNumberBase<T,I,MetaPhysicL::DynamicSparseNumberVector>(src) {}


template <typename T, typename I>
template <typename T2, typename I2>
typename MultipliesType<T,T2>::supertype
DynamicSparseNumberVector<T,I>::dot (const DynamicSparseNumberVector<T2,I2>& a) const
{
  typename MultipliesType<T,T2>::supertype returnval = 0;

  for (I i1 = 0; i1 != this->_indices.size(); ++i1)
    {
      typename std::vector<I2>::const_iterator it2 =
        std::lower_bound(a.nude_indices().begin(),
                         a.nude_indices().end(),
                         this->_indices[i1]);

      if (it2 != a.nude_indices().end())
        {
          std::size_t i2 = it2 - a.nude_indices().begin();

          returnval += this->_data[i1] * a.raw_at(i2);
        }
    }

  return returnval;
}

template <typename T, typename I>
template <typename T2, typename I2>
DynamicSparseNumberVector<DynamicSparseNumberVector<
  typename MultipliesType<T,T2>::supertype,
  I2>, I>
DynamicSparseNumberVector<T,I>::outerproduct
  (const DynamicSparseNumberVector<T2, I2>& a) const
{
  DynamicSparseNumberVector<DynamicSparseNumberVector<
    typename MultipliesType<T,T2>::supertype,
    I2>, I> returnval;

  returnval.nude_indices() = this->_indices;

  std::size_t index_size = this->size();
  std::size_t index2_size = a.size();

  returnval.nude_data().resize(index_size);
  for (unsigned int i=0; i != index_size; ++i)
    {
      returnval.raw_at(i).nude_indices() = a.nude_indices();

      returnval.raw_at(i).nude_data().resize(index2_size);
      for (unsigned int j=0; j != index2_size; ++j)
        returnval.raw_at(i).raw_at(j) = this->_data[i] * a.raw_at(j);
    }

  return returnval;
}

template <typename T, typename I>
DynamicSparseNumberVector<DynamicSparseNumberVector<T, I>, I>
DynamicSparseNumberVector<T,I>::identity(std::size_t n)
{
  DynamicSparseNumberVector<DynamicSparseNumberVector<T, I>, I>
    returnval;
  returnval.resize(n);
  for (unsigned int i=0; i != n; ++i)
    {
      returnval.raw_index(i) = i;
      returnval.raw_at(i).nude_indices().resize(1, i);
      returnval.raw_at(i).nude_data().resize(1, 1);
    }
  return returnval;
}


//
// Non-member functions
//


template <typename T, typename I, typename I2>
inline
DynamicSparseNumberVector<DynamicSparseNumberVector<T, I>, I2>
transpose(const DynamicSparseNumberVector<DynamicSparseNumberVector<T, I2>, I>& /*a*/)
{
  DynamicSparseNumberVector<DynamicSparseNumberVector<T, I>, I2> returnval;

  metaphysicl_not_implemented();

  return returnval;
}


template <typename T, typename I>
T
sum (const DynamicSparseNumberVector<T, I> &a)
{
  std::size_t index_size = a.size();

  T returnval = 0;

  for (unsigned int i=0; i != index_size; ++i) {
    returnval += a.raw_at(i);
  }

  return returnval;
}


DynamicSparseNumberBase_op(DynamicSparseNumberVector, +, Plus)       // Union)
DynamicSparseNumberBase_op(DynamicSparseNumberVector, -, Minus)      // Union)
DynamicSparseNumberBase_op(DynamicSparseNumberVector, *, Multiplies) // Intersection)
DynamicSparseNumberBase_op(DynamicSparseNumberVector, /, Divides)    // First)


template <typename T, typename I>
inline
typename RawType<DynamicSparseNumberVector<T, I> >::value_type
RawType<DynamicSparseNumberVector<T, I> >::value(const DynamicSparseNumberVector<T, I>& a)
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


#endif // METAPHYSICL_DYNAMICSPARSENUMBERVECTOR_H
