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

#ifndef METAPHYSICL_DUALSPARSENUMBERSTRUCT_H
#define METAPHYSICL_DUALSPARSENUMBERSTRUCT_H


#include "metaphysicl/dualnumber.h"
#include "metaphysicl/sparsenumberstruct.h"


namespace MetaPhysicL {

template <typename HeadType, typename TailSet, typename Comparison>
struct DerivativeType<MetaPhysicL::Container<HeadType,TailSet,Comparison> >
{
  typedef
    MetaPhysicL::Container<
      typename HeadType::template rebind<
        typename DerivativeType<typename HeadType::data_type>::type
      >::other,
      typename DerivativeType<TailSet>::type,
      Comparison
    > type;
};


template <typename HeadType, typename TailSet, typename Comparison>
struct DerivativesType<MetaPhysicL::Container<HeadType,TailSet,Comparison> >
{
  typedef
    MetaPhysicL::Container<
      typename HeadType::template rebind<
        typename DerivativesType<typename HeadType::data_type>::type
      >::other,
      typename DerivativesType<TailSet>::type,
      Comparison
    > type;
};


template <typename NullHeadType>
struct DerivativeType<MetaPhysicL::NullContainer<NullHeadType> >
{
  typedef MetaPhysicL::NullContainer<NullHeadType> type;
};


template <typename NullHeadType>
struct DerivativesType<MetaPhysicL::NullContainer<NullHeadType> >
{
  typedef MetaPhysicL::NullContainer<NullHeadType> type;
};


template <typename IndexSet>
struct DerivativeType<SparseNumberStruct<IndexSet> >
{
  typedef SparseNumberStruct<typename DerivativeType<IndexSet>::type> type;
};


template <typename IndexSet>
struct DerivativesType<SparseNumberStruct<IndexSet> >
{
  typedef SparseNumberStruct<typename DerivativesType<IndexSet>::type> type;
};


struct RuntimeDerivativeSubfunctor
{
  template <typename T1>
  typename DerivativeType<T1>::type
  operator()(const T1& x, unsigned int derivativeindex) const {
    return derivative(x, derivativeindex);
  }
};


struct DerivativesSubfunctor
{
  template <typename T1>
  typename DerivativeType<T1>::type
  operator()(const T1& x, unsigned int derivativeindex) const {
    return derivative(x, derivativeindex);
  }
};


template <typename IndexSet>
inline
typename DerivativeType<SparseNumberStruct<IndexSet> >::type
derivative (const SparseNumberStruct<IndexSet>& a,
            unsigned int derivativeindex)
{
  typedef typename DerivativeType<IndexSet>::type IS;
  SparseNumberStruct<IS> returnval;

  typename IndexSet::ForEach()
    (BinaryFunctor<RuntimeDerivativeSubfunctor, IndexSet, ConstantDataSet<unsigned int>, IS>
      (RuntimeDerivativeSubfunctor(), a.raw_data(), ConstantDataSet<unsigned int>(derivativeindex), returnval.raw_data()));

  return returnval;
}


template <typename IndexSet>
inline
typename DerivativesType<SparseNumberStruct<IndexSet> >::type
derivative (const SparseNumberStruct<IndexSet>& a)
{
  typedef typename DerivativesType<IndexSet>::type IS;
  SparseNumberStruct<IS> returnval;

  typename IndexSet::ForEach()
    (UnaryFunctor<DerivativesSubfunctor, IndexSet, IS>
      (DerivativesSubfunctor(), a.raw_data(), returnval.raw_data()));

  return returnval;
}


template <unsigned int derivativeindex>
struct DerivativeSubfunctor
{
  template <typename T1>
  typename DerivativeType<T1>::type
  operator()(const T1& x) const {
    return DerivativeOf<T1, derivativeindex>::derivative(x);
  }
};


template <typename IndexSet, unsigned int derivativeindex>
struct DerivativeOf<SparseNumberStruct<IndexSet>, derivativeindex>
{
  static
  typename DerivativeType<SparseNumberStruct<IndexSet> >::type
  derivative (const SparseNumberStruct<IndexSet>& a)
  {
    typedef typename DerivativeType<IndexSet>::type IS;
    SparseNumberStruct<IS> returnval;

    typename IndexSet::ForEach()
      (UnaryFunctor<DerivativeSubfunctor<derivativeindex>, IndexSet, IS>
        (DerivativeSubfunctor<derivativeindex>(), a.raw_data(), returnval.raw_data()));

    return returnval;
  }
};


template <typename IndexSet, typename Tout>
struct DivergenceFunctor
{
  DivergenceFunctor(const IndexSet &in, Tout& out) : _in(in), _out(out) {}

  template <typename ValueType>
  void operator()() const {
    _out += DerivativeOf<typename ValueType::data_type,
                         ValueType::value>::derivative
      (_in.template data<ValueType>());
  }

private:
  const IndexSet& _in;
  Tout& _out;
};


// For a vector of values a[i] each of which has a defined gradient,
// the divergence is the sum of derivative_wrt_xi(a[i])

// For a tensor of values, we take the divergence with respect to the
// first index.
template <typename IndexSet>
inline
typename DerivativeType<typename MetaPhysicL::ContainerSupertype<IndexSet>::type>::type
divergence(const SparseNumberStruct<IndexSet>& a)
{
  typedef typename 
    DerivativeType<
      typename MetaPhysicL::ContainerSupertype<IndexSet>::type
    >::type
    return_type;
  return_type returnval = 0;

  typename IndexSet::ForEach()
    (DivergenceFunctor<IndexSet,return_type>
      (a.raw_data(), returnval));

  return returnval;
}


template <typename IndexSet>
struct SetGradient
{
  typedef
    MetaPhysicL::Container<
      typename IndexSet::head_type::template rebind<
        typename IndexSet::head_type::data_type::derivatives_type
      >::other,
      typename SetGradient<typename IndexSet::tail_set>::type,
      typename IndexSet::comparison
    > type;
};


template <typename NullHeadType>
struct SetGradient<MetaPhysicL::NullContainer<NullHeadType> >
{
  typedef MetaPhysicL::NullContainer<NullHeadType> type;
};


struct GradientSubfunctor {
  template <typename T>
  typename T::derivatives_type
  operator()(const T& x) const { return x.derivatives(); }
};


// For a vector of values, the gradient is going to be a tensor
template <typename IndexSet>
inline
SparseNumberStruct<typename SetGradient<IndexSet>::type>
gradient(const SparseNumberStruct<IndexSet>& a)
{
  typedef typename SetGradient<IndexSet>::type IS;
  SparseNumberStruct<IS> returnval;

  typename IndexSet::ForEach()
    (UnaryFunctor<GradientSubfunctor,IndexSet,IS>
      (GradientSubfunctor(), a.raw_data(), returnval.raw_data()));

  return returnval;
}

// DualNumber is subordinate to SparseNumberStruct,
// NullType is subordinate to DualNumber,
// DualNumber is subordinate to IndexSet,



#define DualSparseNumberStruct_comparisons(templatename) \
template<typename T, typename D, typename IndexSet, bool reverseorder> \
struct templatename<SparseNumberStruct<IndexSet>, DualNumber<T, D>, reverseorder> { \
  typedef SparseNumberStruct<typename Symmetric##templatename<IndexSet, DualNumber<T, D>, reverseorder>::supertype> supertype; \
}; \
 \
template<typename T, typename D, bool reverseorder> \
struct templatename<DualNumber<T, D>, MetaPhysicL::NullType, reverseorder> { \
  typedef DualNumber<T, D> supertype; \
}; \
 \
template<typename T, typename D, typename HeadType, typename TailSet, typename Comparison, bool reverseorder> \
struct templatename<MetaPhysicL::Container<HeadType,TailSet,Comparison>, DualNumber<T,D>, reverseorder> { \
  typedef typename \
    MetaPhysicL::Container<HeadType,TailSet,Comparison>::template UpgradeType<DualNumber<T, D> >::type \
      supertype; \
}; \
 \
template<typename NullHeadType, typename T, typename D, bool reverseorder> \
struct templatename<MetaPhysicL::NullContainer<NullHeadType>, DualNumber<T, D>, reverseorder> { \
  typedef MetaPhysicL::NullContainer<NullHeadType> supertype; \
}

DualSparseNumberStruct_comparisons(CompareTypes);
DualSparseNumberStruct_comparisons(PlusType);
DualSparseNumberStruct_comparisons(MinusType);
DualSparseNumberStruct_comparisons(MultipliesType);
DualSparseNumberStruct_comparisons(DividesType);
DualSparseNumberStruct_comparisons(AndType);
DualSparseNumberStruct_comparisons(OrType);

} // namespace MetaPhysicL

#endif // METAPHYSICL_DUALSPARSENUMBERSTRUCT_H
