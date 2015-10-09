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

#ifndef METAPHYSICL_DUALNAMEDARRAY_H
#define METAPHYSICL_DUALNAMEDARRAY_H


#include "metaphysicl/dualexpression.h"
#include "metaphysicl/namedindexarray.h"


namespace MetaPhysicL {

template <typename DataVector, typename SparseSizeVector>
struct DerivativeType<NamedIndexArray<DataVector, SparseSizeVector> >
{
  typedef NamedIndexArray<typename DerivativeType<DataVector>::type,
                          SparseSizeVector> type;
};


template <typename DataVector, typename SparseSizeVector>
struct DerivativesType<NamedIndexArray<DataVector, SparseSizeVector> >
{
  typedef NamedIndexArray<typename DerivativesType<DataVector>::type,
                          SparseSizeVector> type;
};


template <typename DataVector, typename SparseSizeVector>
inline
typename DerivativeType<NamedIndexArray<DataVector, SparseSizeVector> >::type
derivative(const NamedIndexArray<DataVector, SparseSizeVector>& a, unsigned int derivativeindex)
{
  typename DerivativeType<NamedIndexArray<DataVector, SparseSizeVector> >::type returnval;
  for (unsigned int i=0; i != a.size(); ++i)
    returnval.raw_data()[i] = derivative(a.raw_data()[i], derivativeindex);

  return returnval;
}

template <typename DataVector, typename SparseSizeVector>
inline
typename DerivativesType<NamedIndexArray<DataVector, SparseSizeVector> >::type
derivatives(const NamedIndexArray<DataVector, SparseSizeVector>& a)
{
  typename DerivativesType<NamedIndexArray<DataVector, SparseSizeVector> >::type returnval;
  for (unsigned int i=0; i != a.size(); ++i)
    returnval.raw_data()[i] = derivatives(a.raw_data()[i]);

  return returnval;
}


template <typename DataVector, typename SparseSizeVector, int derivativeindex>
struct DerivativeOf<NamedIndexArray<DataVector, SparseSizeVector>, derivativeindex>
{
  static
  typename DerivativeType<NamedIndexArray<DataVector, SparseSizeVector> >::type
  derivative(const NamedIndexArray<DataVector, SparseSizeVector>& a)
  {
    typename DerivativeType<NamedIndexArray<DataVector, SparseSizeVector> >::type returnval;
    for (unsigned int i=0; i != a.size(); ++i)
      returnval.raw_data()[i] =
        DerivativeOf<decltype(a.raw_data()[i]),derivativeindex>::derivative(a.raw_data()[i]);
  
    return returnval;
  }
};


// For an array of values a[i] each of which has a defined divergence,
// the divergence is the array of divergence(a[i])

template <typename DataVector, typename SparseSizeVector>
inline
typename DerivativeType<NamedIndexArray<DataVector, SparseSizeVector> >::type
divergence(const NamedIndexArray<DataVector, SparseSizeVector>& a)
{
  typename DerivativeType<NamedIndexArray<DataVector, SparseSizeVector> >::type
    returnval = 0;

  for (unsigned int i=0; i != a.size(); ++i)
    returnval.raw_data()[i] = divergence(a.raw_data()[i]);

  return returnval;
}


// For a vector of values, the gradient is going to be a vector of gradients
template <typename DataVector, typename SparseSizeVector>
inline
NamedIndexArray<typename DerivativeType<DataVector>::type, SparseSizeVector>
gradient(const NamedIndexArray<DataVector, SparseSizeVector>& a)
{
  NamedIndexArray<typename DerivativeType<DataVector>::type,
                  SparseSizeVector>
    returnval;

  for (unsigned int i=0; i != a.size(); ++i)
    returnval.raw_data()[i] = gradient(a.raw_data()[i]);

  return returnval;
}

// NamedIndexArray is subordinate to DualExpression

// FIXME - can we put this under selective user control some how?  The
// opposite rule makes sense in some cases.

#define DualNamedArray_comparisons(templatename) \
template<typename T, typename D, \
         typename DataVector, typename SparseSizeVector, bool reverseorder> \
struct templatename< \
  DualExpression<T, D>, \
  NamedIndexArray<DataVector, SparseSizeVector>, \
  reverseorder> { \
  typedef DualExpression< \
    NamedIndexArray< \
      typename Symmetric##templatename<DataVector, T, reverseorder>::supertype, \
      SparseSizeVector>, \
    NamedIndexArray< \
      typename Symmetric##templatename<DataVector, D, reverseorder>::supertype, \
      SparseSizeVector> \
  > supertype; \
}

DualNamedArray_comparisons(CompareTypes);
// DualNamedArray_comparisons(PlusType);
// DualNamedArray_comparisons(MinusType);
// DualNamedArray_comparisons(MultipliesType);
// DualNamedArray_comparisons(DividesType);
// DualNamedArray_comparisons(AndType);
// DualNamedArray_comparisons(OrType);

} // namespace MetaPhysicL

#endif // METAPHYSICL_DUALNAMEDARRAY_H
