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

#ifndef METAPHYSICL_DUALSPARSENUMBERVECTOR_H
#define METAPHYSICL_DUALSPARSENUMBERVECTOR_H


#include "metaphysicl/dualnumber.h"
#include "metaphysicl/sparsenumbervector.h"


namespace MetaPhysicL {

template <typename T, typename IndexSet>
struct DerivativeType<SparseNumberVector<T, IndexSet> >
{
  typedef SparseNumberVector<typename DerivativeType<T>::type, IndexSet> type;
};


template <typename T, typename IndexSet>
struct DerivativesType<SparseNumberVector<T, IndexSet> >
{
  typedef SparseNumberVector<typename DerivativesType<T>::type, IndexSet> type;
};


template <typename T, typename IndexSet>
inline
typename DerivativeType<SparseNumberVector<T, IndexSet> >::type
derivative (const SparseNumberVector<T, IndexSet>& a,
            unsigned int derivativeindex)
{
  typename DerivativeType<SparseNumberVector<T, IndexSet> >::type returnval;
  for (unsigned int i=0; i != IndexSet::size; ++i)
    returnval.raw_at(i) = derivative(a.raw_at(i),derivativeindex);
  return returnval;
}


template <typename T, typename IndexSet>
inline
typename DerivativesType<SparseNumberVector<T, IndexSet> >::type
derivatives (const SparseNumberVector<T, IndexSet>& a)
{
  typename DerivativesType<SparseNumberVector<T, IndexSet> >::type returnval;
  for (unsigned int i=0; i != IndexSet::size; ++i)
    returnval.raw_at(i) = derivatives(a.raw_at(i));
  return returnval;
}


template <typename T, typename IndexSet, unsigned int derivativeindex>
struct DerivativeOf<SparseNumberVector<T, IndexSet>, derivativeindex>
{
  static
  typename DerivativeType<SparseNumberVector<T, IndexSet> >::type
  derivative (const SparseNumberVector<T, IndexSet>& a)
  {
    typename DerivativeType<SparseNumberVector<T, IndexSet> >::type returnval;
    for (unsigned int i=0; i != IndexSet::size; ++i)
      returnval.raw_at(i) = DerivativeOf<T,derivativeindex>::derivative(a.raw_at(i));
    return returnval;
  }
};


template <typename T, typename IndexSet>
struct DivergenceVectorFunctor {
  DivergenceVectorFunctor
    (const T* in, typename DerivativeType<T>::type& out) :
      _in(in), _out(out) { out = 0; }
    
  template <typename ValueType>
  inline void operator()() const {
    const unsigned int
      indexin = IndexSet::template IndexOf<ValueType>::index;
    _out += DerivativeOf<T, ValueType::value>::derivative(_in[indexin]);
  }

private:
  const T* _in;
  typename DerivativeType<T>::type& _out;
};


// For a vector of values a[i] each of which has a defined gradient,
// the divergence is the sum of derivative_wrt_xi(a[i])

// For a tensor of values, we take the divergence with respect to the
// first index.
template <typename T, typename IndexSet>
inline
typename DerivativeType<T>::type
divergence(const SparseNumberVector<T, IndexSet>& a)
{
  typename DerivativeType<T>::type returnval = 0;

  typename IndexSet::ForEach()
    (DivergenceVectorFunctor<T,IndexSet>(a.raw_data(), returnval));

  return returnval;
}


// For a vector of values, the gradient is going to be a tensor
template <typename T, typename IndexSet>
inline
SparseNumberVector<typename T::derivatives_type, IndexSet>
gradient(const SparseNumberVector<T, IndexSet>& a)
{
  static const unsigned int size = IndexSet::size;

  SparseNumberVector<typename T::derivatives_type, IndexSet> returnval;

  for (unsigned int i=0; i != size; ++i)
    returnval.raw_at(i) = gradient(a.raw_at(i));

  return returnval;
}

// DualNumber is subordinate to SparseNumberVector

#define DualSparseNumberVector_comparisons(templatename) \
template<typename T, typename T2, typename D, typename IndexSet, bool reverseorder> \
struct templatename<SparseNumberVector<T2, IndexSet>, DualNumber<T, D>, reverseorder> { \
  typedef SparseNumberVector<typename Symmetric##templatename<T2,DualNumber<T, D>,reverseorder>::supertype, IndexSet> supertype; \
}

DualSparseNumberVector_comparisons(CompareTypes);
DualSparseNumberVector_comparisons(PlusType);
DualSparseNumberVector_comparisons(MinusType);
DualSparseNumberVector_comparisons(MultipliesType);
DualSparseNumberVector_comparisons(DividesType);
DualSparseNumberVector_comparisons(AndType);
DualSparseNumberVector_comparisons(OrType);

} // namespace MetaPhysicL

#endif // METAPHYSICL_DUALSPARSENUMBERVECTOR_H
