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


#ifndef METAPHYSICL_SPARSENUMBERSTRUCT_H
#define METAPHYSICL_SPARSENUMBERSTRUCT_H

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <ostream>

#include "metaphysicl/compare_types.h"
#include "metaphysicl/ct_set.h"
#include "metaphysicl/raw_type.h"
#include "metaphysicl/testable.h"

namespace MetaPhysicL {

// Forward declarations
template <typename IndexSet>
class SparseNumberStruct;


  template <typename SubFunctor, typename SetIn, typename SetOut>
  struct UnaryFunctor {
    UnaryFunctor(SubFunctor f, const SetIn &in, SetOut &out) :
      _subfunctor(f), _in(in), _out(out) {}

    template <typename ValueType>
    void operator()() const {
      _out.template data<ValueType>() =
        _subfunctor(_in.template data<ValueType>());
    }

  private:
    SubFunctor _subfunctor;
    const SetIn&  _in;
    SetOut& _out;
  };


// Lots of functional objects exist in std:: but with the templating around the
// class rather than around operator(), so we need new functors for heterogenous containers.

  struct NegateSubfunctor {
    template <typename T>
    T operator()(const T& x) const { return -x; }
  };

  struct PlusSubfunctor {
    template <typename T1, typename T2>
    typename SymmetricPlusType<T1,T2>::supertype
    operator()(const T1& x, const T2& y) const { return x + y; }
  };

  struct MinusSubfunctor {
    template <typename T1, typename T2>
    typename SymmetricMinusType<T1,T2>::supertype
    operator()(const T1& x, const T2& y) const { return x - y; }
  };

  struct MultipliesSubfunctor {
    template <typename T1, typename T2>
    typename SymmetricMultipliesType<T1,T2>::supertype
    operator()(const T1& x, const T2& y) const { return x * y; }
  };

  struct DividesSubfunctor {
    template <typename T1, typename T2>
    typename SymmetricDividesType<T1,T2>::supertype
    operator()(const T1& x, const T2& y) const { return x / y; }
  };

  struct AndSubfunctor {
    template <typename T1, typename T2>
    typename SymmetricAndType<T1,T2>::supertype
    operator()(const T1& x, const T2& y) const { return x && y; }
  };

  struct OrSubfunctor {
    template <typename T1, typename T2>
    typename SymmetricOrType<T1,T2>::supertype
    operator()(const T1& x, const T2& y) const { return x || y; }
  };


  template <typename SubFunctor, typename SetIn1, typename SetIn2, typename SetOut>
  struct BinaryFunctor {
    BinaryFunctor(SubFunctor f, const SetIn1 &in1, const SetIn2 &in2, SetOut &out) :
      _subfunctor(f), _in1(in1), _in2(in2), _out(out) {}

    template <typename ValueType>
    inline void operator()() const {
      _out.template data<ValueType>() =
        _subfunctor(_in1.template data<ValueType>(),
                    _in2.template data<ValueType>());
    }

  private:
    SubFunctor _subfunctor;
    const SetIn1& _in1;
    const SetIn2& _in2;
    SetOut& _out;
  };


  template <typename T>
  struct ConstantDataSet
  {
    ConstantDataSet(const T& t) : _data(t) {}

    template <typename ValueType>
    const T& data() const { return _data; }

    // A ConstantDataSet contains any requested index.
    template <typename ValueType>
    struct Contains {
      static const bool value = true;
    };

  private:
    const T& _data;
  };


template <typename IndexSet>
struct SumType<SparseNumberStruct<IndexSet> >
{
  typedef typename 
    SymmetricPlusType<
      typename IndexSet::head_type::data_type,
      typename SumType<
        SparseNumberStruct<typename IndexSet::tail_set>
      >::supertype
    >::supertype supertype;
};


template <typename NullHeadType>
struct SumType<SparseNumberStruct<MetaPhysicL::NullContainer<NullHeadType> > >
{
  typedef MetaPhysicL::NullType supertype;
};


template <typename Set1, typename Set2>
struct DotType<SparseNumberStruct<Set1>, SparseNumberStruct<Set2> >
{
  typedef typename Set1::template Intersection<
    typename Set2::template rebind<MetaPhysicL::NullType>::other>::type
    IntersectionSet1;
  typedef typename Set2::template Intersection<
    typename Set1::template rebind<MetaPhysicL::NullType>::other>::type
    IntersectionSet2;

  typedef typename SymmetricPlusType<
    typename SymmetricMultipliesType<
      typename IntersectionSet1::head_type::data_type,
      typename IntersectionSet2::head_type::data_type
    >::supertype,
    typename DotType<
      SparseNumberStruct<typename IntersectionSet1::tail_set>,
      SparseNumberStruct<typename IntersectionSet2::tail_set>
    >::supertype
  >::supertype supertype;
};


template <typename Set1, typename NullHeadType>
struct DotType<SparseNumberStruct<Set1>,
               SparseNumberStruct<MetaPhysicL::NullContainer<NullHeadType> > >
{
  typedef MetaPhysicL::NullType supertype;
};


template <typename NullHeadType, typename Set2>
struct DotType<SparseNumberStruct<MetaPhysicL::NullContainer<NullHeadType> >,
               SparseNumberStruct<Set2> >
{
  typedef MetaPhysicL::NullType supertype;
};


template <typename NullHeadType1, typename NullHeadType2>
struct DotType<SparseNumberStruct<MetaPhysicL::NullContainer<NullHeadType1> >,
               SparseNumberStruct<MetaPhysicL::NullContainer<NullHeadType2> > >
{
  typedef MetaPhysicL::NullType supertype;
};


template <typename Set1, typename Set2>
struct OuterProductType<SparseNumberStruct<Set1>, SparseNumberStruct<Set2> >
{
  typedef 
    SparseNumberStruct<
      MetaPhysicL::Container<
        typename Set1::head_type::template rebind<
          typename SymmetricMultipliesType<
	    typename Set1::head_type::data_type, SparseNumberStruct<Set2> >::supertype
        >::other,
        typename OuterProductType<
          SparseNumberStruct<typename Set1::tail_set>,
          SparseNumberStruct<Set2> >::supertype::index_set,
        typename Set1::comparison
      >
    > supertype;
};


template <typename NullHeadType, typename Set2>
struct OuterProductType<SparseNumberStruct<MetaPhysicL::NullContainer<NullHeadType> >,
                        SparseNumberStruct<Set2> >
{
  typedef SparseNumberStruct<MetaPhysicL::NullContainer<NullHeadType> > supertype;
};


template <typename Set1>
struct SetDiagonalTensor
{
  typedef 
    MetaPhysicL::Container<
      typename Set1::head_type::template rebind<
        SparseNumberStruct<
          MetaPhysicL::Container<
            typename Set1::head_type,
            MetaPhysicL::NullContainer<typename Set1::head_type>
          >
        >
      >::other,
      typename SetDiagonalTensor<typename Set1::tail_set>::type
    > type;
};


template <typename NullHeadType>
struct SetDiagonalTensor<MetaPhysicL::NullContainer<NullHeadType> >
{
  typedef MetaPhysicL::NullContainer<NullHeadType> type;
};


template <typename IndexSet>
class SparseNumberStruct : public safe_bool<SparseNumberStruct<IndexSet> >
{
public:
  typedef IndexSet index_set;

  typedef typename MetaPhysicL::ContainerSupertype<IndexSet>::type value_type;

  template <unsigned int i>
  struct entry_type {
    typedef typename IndexSet::template ElementOf<MetaPhysicL::UnsignedIntType<i> >::type::data_type type;
  };

  template <typename T>
  struct rebind {
    typedef SparseNumberStruct<typename IndexSet::template rebind<T>::other> other;
  };

  template <typename Functor>
  static void for_each_datum(const Functor &f) {
    typename IndexSet::ForEach()(f);
  }

  static const unsigned int index_size = IndexSet::size;

  SparseNumberStruct() {}

  template <bool, typename ValueType, typename SetIn>
  struct SubCopyFunctor {
    inline static void apply(const SetIn& in, IndexSet &out) {
      out.template data<ValueType>() =
        in.template data<ValueType>();
    }
  };

  template <typename ValueType, typename SetIn>
  struct SubCopyFunctor<false, ValueType, SetIn> {
    inline static void apply(const SetIn&, IndexSet &out) {
      out.template data<ValueType>() = 0;
    }
  };

  template <typename SetIn>
  struct CopyFunctor {
    CopyFunctor(const SetIn& in, IndexSet& out) : _in(in), _out(out) {}

    template <typename ValueType>
    inline void operator()() const {
      SubCopyFunctor<SetIn::template Contains<ValueType>::value,
               ValueType, SetIn>::apply(_in, _out);
    }

  private:
    const SetIn& _in;
    IndexSet& _out;
  };

  template <typename SubFunctor, typename SetIn>
  struct OpEqualsFunctor {
    OpEqualsFunctor(SubFunctor f, const SetIn& in, IndexSet& out) :
      _subfunctor(f), _in(in), _out(out) {}

    template <typename ValueType>
    inline void operator()() const {
      _out.template data<ValueType>() =
        _subfunctor(_out.template data<ValueType>(),
                    _in.template data<ValueType>());
    }

  private:
    SubFunctor _subfunctor;
    const SetIn& _in;
    IndexSet& _out;
  };


  template <typename SubFunctor, typename Tout>
  struct UnaryIteratedFunctor {
    UnaryIteratedFunctor(SubFunctor f, const IndexSet& in, Tout& out) :
      _subfunctor(f), _in(in), _out(out) {}

    template <typename ValueType>
    inline void operator()() const {
      _subfunctor(_in.template data<ValueType>(), _out);
    }

  private:
    SubFunctor _subfunctor;
    const IndexSet& _in;
    Tout& _out;
  };


  template <typename SubFunctor, typename IndexSet2, typename Tout>
  struct BinaryIteratedFunctor {
    BinaryIteratedFunctor(SubFunctor f, const IndexSet& in1, const IndexSet2& in2, Tout& out) :
      _subfunctor(f), _in1(in1), _in2(in2), _out(out) {}

    template <typename ValueType>
    inline void operator()() const {
      _subfunctor(_in1.template data<ValueType>(),
                  _in2.template data<ValueType>(), _out);
    }

  private:
    SubFunctor _subfunctor;
    const IndexSet& _in1;
    const IndexSet2& _in2;
    Tout& _out;
  };

  struct AccumulateOr {
    template <typename T1>
    inline void operator()(const T1& in1, bool& out) const
      { out = out || in1; }
  };

  struct AccumulateSum {
    template <typename T1, typename Tout>
    inline void operator()(const T1& in, Tout& out) const
      { out += in; }
  };

  struct AccumulateDot {
    template <typename T1, typename T2, typename Tout>
    inline void operator()(const T1& in1, const T2& in2, Tout& out) const
      { out += in1 * in2; }
  };

  template <typename T2>
  SparseNumberStruct(const T2& val) {
/*
    // This makes no sense unless val is 0!
    // Except that we're abusing this constructor in identity()
#ifndef NDEBUG
    if (val)
      throw std::domain_error("Cannot initialize SparseNumberStruct with non-zero scalar");
#endif
*/
    for_each_datum(CopyFunctor<ConstantDataSet<T2> > (ConstantDataSet<T2>(val), _data));
  }


  // We can have an implicit constructor that gives a compile-time
  // error for IndexSet2 which is not a subset of IndexSet
  template <typename IndexSet2>
  SparseNumberStruct(SparseNumberStruct<IndexSet2> src) {
    for_each_datum(CopyFunctor<IndexSet2>(src.raw_data(), _data));
    ctassert<IndexSet2::template Difference<IndexSet>::type::size == 0>::apply();
  }

  // We have a dangerous, explicit named constructor that silently
  // drops data for IndexSet2 which is not a subset of IndexSet
  template <typename IndexSet2>
  static SparseNumberStruct slice(SparseNumberStruct<IndexSet2> src) {
    SparseNumberStruct returnval;
    typename IndexSet::template Intersection<IndexSet2>::type::ForEach()
      (CopyFunctor<IndexSet2>(src.raw_data(), returnval.raw_data()));
    typename IndexSet::template Difference<IndexSet2>::type::ForEach()
      (CopyFunctor<MetaPhysicL::NullContainer<
         typename IndexSet2::head_type
       > >
        (MetaPhysicL::NullContainer<
           typename IndexSet2::head_type
         >(),
         returnval.raw_data()));
    return returnval;
  }


  IndexSet& raw_data()
    { return _data; }

  const IndexSet& raw_data() const
    { return _data; }


// FIXME - is it even possible to make these work properly on
// heterogenous containers?
/*
  typename IndexSet::head_type::data_type& operator[](const typename IndexSet::value_type& i)
    { return _data.runtime_data(i); }

  const typename IndexSet::head_type::data_type& operator[](const typename IndexSet::value_type& i) const
    { return _data.runtime_data(i); }
*/

  template <unsigned int i>
  typename entry_type<i>::type& get()
    { return _data.template data<MetaPhysicL::UnsignedIntType<i> >(); }

  template <unsigned int i>
  const typename entry_type<i>::type& get() const
    { return _data.template data<MetaPhysicL::UnsignedIntType<i> >(); }

  template <unsigned int i>
  typename entry_type<i>::type& insert()
    { return _data.template data<MetaPhysicL::UnsignedIntType<i> >(); }

  std::size_t size() const
    { return IndexSet::size; }

  bool boolean_test() const {
    // FIXME - we need a short-circuitable for_each_datum
    bool is_nonzero = false;
    for_each_datum (UnaryIteratedFunctor<AccumulateOr, bool>
      (AccumulateOr(), _data, is_nonzero));
    return is_nonzero;
  }

  SparseNumberStruct<IndexSet> operator- () const {
    SparseNumberStruct<IndexSet> returnval;
    for_each_datum (UnaryFunctor<NegateSubfunctor,IndexSet,IndexSet>
      (NegateSubfunctor(), _data, returnval.raw_data()));
    return returnval;
  }

  // Not defineable since !0 != 0
  // SparseNumberStruct<IndexSet> operator- () const;

  template <typename IndexSet2>
  SparseNumberStruct<IndexSet>&
    operator+= (const SparseNumberStruct<IndexSet2>& a) { 
    typename IndexSet2::ForEach()
      (OpEqualsFunctor<PlusSubfunctor, IndexSet2>
        (PlusSubfunctor(), a.raw_data(), _data));
    ctassert<IndexSet2::template Difference<IndexSet>::type::size == 0>::apply();
    return *this;
  }

  template <typename IndexSet2>
  SparseNumberStruct<IndexSet>&
    operator-= (const SparseNumberStruct<IndexSet2>& a) { 
    typename IndexSet2::ForEach()
      (OpEqualsFunctor<MinusSubfunctor, IndexSet2>
        (MinusSubfunctor(), a.raw_data(), _data));
    ctassert<IndexSet2::template Difference<IndexSet>::type::size == 0>::apply();
    return *this;
  }

  template <typename IndexSet2>
  SparseNumberStruct<IndexSet>&
    operator*= (const SparseNumberStruct<IndexSet2>& a) { 
    typename IndexSet::template Intersection<IndexSet2>::type::ForEach()
      (OpEqualsFunctor<MultipliesSubfunctor, IndexSet2>
        (MultipliesSubfunctor(), a.raw_data(), _data));
    typename IndexSet::template Difference<IndexSet2>::type::ForEach()
      (CopyFunctor<MetaPhysicL::NullContainer<
         typename IndexSet2::head_type
       > >
        (MetaPhysicL::NullContainer<typename IndexSet2::head_type>(),
         _data));
    return *this;
  }

  template <typename IndexSet2>
  SparseNumberStruct<IndexSet>&
    operator/= (const SparseNumberStruct<IndexSet2>& a) { 
    for_each_datum (OpEqualsFunctor<DividesSubfunctor, IndexSet2>
      (DividesSubfunctor(), a.raw_data(), _data));
    ctassert<IndexSet::template Difference<IndexSet2>::type::size == 0>::apply();
    return *this;
  }

  template <typename T2>
  SparseNumberStruct<IndexSet>& operator*= (const T2& a) {
    for_each_datum(OpEqualsFunctor<MultipliesSubfunctor, ConstantDataSet<T2> >
      (MultipliesSubfunctor(), ConstantDataSet<T2>(a), _data));
    return *this;
  }

  template <typename T2>
  SparseNumberStruct<IndexSet>& operator/= (const T2& a) {
    for_each_datum(OpEqualsFunctor<DividesSubfunctor, ConstantDataSet<T2> >
      (DividesSubfunctor(), ConstantDataSet<T2>(a), _data));
    return *this;
  }

  template <typename IndexSet2>
  typename DotType<SparseNumberStruct<IndexSet>,
	           SparseNumberStruct<IndexSet2> >::supertype
  dot (const SparseNumberStruct<IndexSet2>& a) const
  {
    typedef typename
      DotType<SparseNumberStruct<IndexSet>,
              SparseNumberStruct<IndexSet2> >::supertype type;
    type returnval = 0;
    typename IndexSet::template Intersection<IndexSet2>::type::ForEach()
      (BinaryIteratedFunctor<AccumulateDot, IndexSet2, type>
        (AccumulateDot(), _data, a.raw_data(), returnval));

    return returnval;
  }

  template <typename IndexSet2>
  typename OuterProductType<SparseNumberStruct<IndexSet>,SparseNumberStruct<IndexSet2> >::supertype
  outerproduct (const SparseNumberStruct<IndexSet2>& a) const
  {
    typedef SparseNumberStruct<IndexSet2> S2;
    typedef typename
      OuterProductType<SparseNumberStruct<IndexSet>,S2>::supertype RV;
    RV returnval;

    typename RV::index_set::ForEach()
      (BinaryFunctor<MultipliesSubfunctor, IndexSet,
       ConstantDataSet<S2>, typename RV::index_set>
        (MultipliesSubfunctor(), _data, ConstantDataSet<S2>(a), returnval.raw_data()));

    return returnval;
  }

  static SparseNumberStruct<typename SetDiagonalTensor<IndexSet>::type>
  identity(std::size_t n=0)
  {
    typedef 
      SparseNumberStruct<typename SetDiagonalTensor<IndexSet>::type> DiagonalStruct;
    DiagonalStruct returnval;
  
    for_each_datum(typename DiagonalStruct::template CopyFunctor<ConstantDataSet<int> >
      (ConstantDataSet<int>(1),returnval.raw_data()));

    return returnval;
  }

  typename SumType<SparseNumberStruct<IndexSet> >::supertype sum () const
  {
    typedef typename SumType<SparseNumberStruct<IndexSet> >::type type;
    type returnval = 0;
    for_each_datum (UnaryIteratedFunctor<AccumulateSum, type>
        (AccumulateSum(), _data, returnval));

    return returnval;
  }

private:
  IndexSet _data;
};


//
// Non-member functions
//

/*
template <typename B, typename T, typename T2,
	  typename IndexSetB, typename IndexSet, typename IndexSet2>
inline
SparseNumberArray<typename SymmetricCompareTypes<T,T2>::supertype,
                  typename IndexSetB::template Union<IndexSet>::type::Union
                    <typename IndexSet2::template Difference<IndexSetB>::type >::type>
if_else (const SparseNumberArray<IndexSetB,B> & condition,
         const SparseNumberArray<IndexSet,T> & if_true,
         const SparseNumberArray<IndexSet2,T2> & if_false)
{
  typedef typename SymmetricCompareTypes<T,T2>::supertype TS;
  typedef typename IndexSetB::template Union<IndexSet>::type::Union
    <typename IndexSet2::template Difference<IndexSetB>::type >::type IndexSetS;

  SparseNumberArray<TS, IndexSetS> returnval;

  FIXME

  return returnval;
}
*/



template <std::size_t size, unsigned int index, typename T>
struct SparseNumberStructUnitVector
{
  typedef MetaPhysicL::Container<
    MetaPhysicL::UnsignedIntType<index, T>,
    MetaPhysicL::NullContainer<MetaPhysicL::UnsignedIntType<index, T> >
  > IndexSet;

  typedef SparseNumberStruct<IndexSet> type;

  static type value() {
    type returnval;
    returnval.raw_data().template data<MetaPhysicL::UnsignedIntType<index> >() = 1;
    return returnval;
  }
};

template <std::size_t size, typename T>
struct SparseNumberStructFullVector
{
  typedef MetaPhysicL::Container<
    MetaPhysicL::UnsignedIntType<size-1, T>,
    typename SparseNumberStructFullVector<size-1,T>::IndexSet
  > IndexSet;

  typedef SparseNumberStruct<IndexSet> type;

  static type value() {
    type returnval;
    returnval.for_each_datum
      (type::template CopyFunctor<ConstantDataSet<int> >
        (ConstantDataSet<int>(1), returnval.raw_data()));
    return returnval;
  }
};

template <typename T>
struct SparseNumberStructFullVector<0,T>
{
  typedef MetaPhysicL::NullContainer<MetaPhysicL::UnsignedIntType<0> > IndexSet;

  typedef SparseNumberStruct<IndexSet> type;

  static type value() {
    type returnval;
    return returnval;
  }
};


template <unsigned int size,
          unsigned int index1=0, typename Data1=void,
          unsigned int index2=0, typename Data2=void,
          unsigned int index3=0, typename Data3=void,
          unsigned int index4=0, typename Data4=void,
          unsigned int index5=0, typename Data5=void,
          unsigned int index6=0, typename Data6=void,
          unsigned int index7=0, typename Data7=void,
          unsigned int index8=0, typename Data8=void>
struct SparseNumberStructOf
{
  template <unsigned int i, typename Data>
  struct UIntOrNullType {
    typedef MetaPhysicL::UnsignedIntType<i, Data> type;
  };

  template <unsigned int i>
  struct UIntOrNullType<i, void> {
    typedef MetaPhysicL::NullContainer<MetaPhysicL::UnsignedIntType<0> > type;
  };

  typedef SparseNumberStruct<
    typename MetaPhysicL::SetConstructor<
      typename UIntOrNullType<index1,Data1>::type,
      typename UIntOrNullType<index2,Data2>::type,
      typename UIntOrNullType<index3,Data3>::type,
      typename UIntOrNullType<index4,Data4>::type,
      typename UIntOrNullType<index5,Data5>::type,
      typename UIntOrNullType<index6,Data6>::type,
      typename UIntOrNullType<index7,Data7>::type,
      typename UIntOrNullType<index8,Data8>::type
    >::type
  > type;
};


template <typename TensorSet>
struct SetColumnValues
{
  typedef typename
  TensorSet::head_type::data_type::index_set::
    template Union<
      typename SetColumnValues<typename TensorSet::tail_set>::type
    >::type type;
};


template <typename NullHeadType>
struct SetColumnValues<MetaPhysicL::NullContainer<NullHeadType> >
{
  typedef MetaPhysicL::NullContainer<NullHeadType> type;
};


template <typename TensorSet, typename ValueType>
struct SetColumn
{
  // FIXME: how to set Comparison?
  typedef typename MetaPhysicL::IfElse<
    TensorSet::head_type::data_type::index_set::template Contains<ValueType>::value,
    MetaPhysicL::Container<
      typename TensorSet::head_type::template rebind<
        typename TensorSet::head_type::data_type::index_set::
          template ElementOf<ValueType>::type::data_type
        >::other,
      typename SetColumn<typename TensorSet::tail_set, ValueType>::type
    >,
    typename SetColumn<typename TensorSet::tail_set, ValueType>::type
  >::type type;
};


template <typename NullHeadType, typename ValueType>
struct SetColumn<MetaPhysicL::NullContainer<NullHeadType>, ValueType>
{
  typedef MetaPhysicL::NullContainer<NullHeadType> type;
};


template <typename TensorSet,
          typename ColumnValues=typename SetColumnValues<TensorSet>::type>
struct SetTensorTranspose
{
  // FIXME: with proper rebind<> we wouldn't need to hardcode
  // SparseNumberStruct here
  // FIXME: how to set Comparison?
  typedef
    MetaPhysicL::Container<
      typename ColumnValues::head_type::template rebind<
        SparseNumberStruct<
          typename SetColumn<
            TensorSet,typename ColumnValues::head_type
          >::type
        >
      >::other,
      typename SetTensorTranspose<
        TensorSet,typename ColumnValues::tail_set
      >::type
    > type;
};


template <typename TensorSet, typename NullHeadType>
struct SetTensorTranspose<TensorSet, MetaPhysicL::NullContainer<NullHeadType> >
{
  typedef MetaPhysicL::NullContainer<NullHeadType> type;
};


template <typename NullHeadType1, typename NullHeadType2>
struct SetTensorTranspose<MetaPhysicL::NullContainer<NullHeadType1>,
                          MetaPhysicL::NullContainer<NullHeadType2> >
{
  typedef MetaPhysicL::NullContainer<NullHeadType1> type;
};


template <typename TensorSetIn, typename TensorSetOut, typename ColumnType>
struct TransposeColumnSubfunctor
{
  TransposeColumnSubfunctor(const TensorSetIn& in, TensorSetOut& out) :
    _in(in), _out(out) {}

  template <typename ValueType>
  inline void operator()() const {
    _out.template data<ColumnType>().raw_data().
         template data<ValueType>() =
      _in.template data<ValueType>().raw_data().
          template data<ColumnType>();
  }

private:
  const TensorSetIn& _in;
  TensorSetOut& _out;
};


template <typename SetIn, typename SetOut>
struct TransposeFunctor
{
  TransposeFunctor(const SetIn& in, SetOut& out) :
    _in(in), _out(out) {}

  template <typename ValueType>
  inline void operator()() const {
    typename SetOut::template ElementOf<ValueType>::type::data_type::index_set::ForEach()
      (TransposeColumnSubfunctor<SetIn, SetOut, ValueType>
        (_in, _out));
  }

private:
  const SetIn& _in;
  SetOut& _out;
};


template <typename TensorIndexSet>
inline
SparseNumberStruct<typename SetTensorTranspose<TensorIndexSet>::type>
transpose(const SparseNumberStruct<TensorIndexSet>& a)
{
  typedef typename SetTensorTranspose<TensorIndexSet>::type TS;
  SparseNumberStruct<TS> returnval;

  typename TensorIndexSet::ForEach()
    (TransposeFunctor<TensorIndexSet, TS>
      (a.raw_data(), returnval.raw_data()));

  return returnval;
}



#define SparseNumberStruct_op_ab(opname, atype, btype, functorname) \
template <typename IndexSet, typename IndexSet2> \
inline \
typename Symmetric##functorname##Type<atype,btype>::supertype \
operator opname (const atype& a, const btype& b) \
{ \
  typedef typename Symmetric##functorname##Type<atype,btype>::supertype type; \
  type returnval = type::slice(a); \
  returnval opname##= b; \
  return returnval; \
}

#define SparseNumberStruct_op(opname, functortype) \
SparseNumberStruct_op_ab(opname, SparseNumberStruct<IndexSet>, SparseNumberStruct<IndexSet2>, functortype)

SparseNumberStruct_op(+, Plus)
SparseNumberStruct_op(-, Minus)
SparseNumberStruct_op(*, Multiplies)
SparseNumberStruct_op(/, Divides)

// Let's also allow scalar times array.
// Scalar plus array, etc. remain undefined in the sparse context.

template <typename T, typename IndexSet>
inline
typename MultipliesType<SparseNumberStruct<IndexSet>,T,true>::supertype
operator * (const T& a, const SparseNumberStruct<IndexSet>& b)
{
  typedef typename MultipliesType<SparseNumberStruct<IndexSet>,T,true>::supertype type;
  type returnval;
  typename IndexSet::ForEach()
    (BinaryFunctor<MultipliesSubfunctor, ConstantDataSet<T>, IndexSet, typename type::index_set>
      (MultipliesSubfunctor(), ConstantDataSet<T>(a), b.raw_data(), returnval.raw_data()));
  return returnval;
}

template <typename T2, typename IndexSet>
inline
typename MultipliesType<SparseNumberStruct<IndexSet>,T2>::supertype
operator * (const SparseNumberStruct<IndexSet>& a, const T2& b)
{
  typedef typename MultipliesType<SparseNumberStruct<IndexSet>,T2>::supertype type;

  typename CompareTypes<SparseNumberStruct<IndexSet>,T2>::supertype
    returnval;
  typename IndexSet::ForEach()
    (BinaryFunctor<MultipliesSubfunctor, IndexSet, ConstantDataSet<T2>, typename type::index_set>
      (MultipliesSubfunctor(), a.raw_data(), ConstantDataSet<T2>(b), returnval.raw_data()));
  return returnval;
}

template <typename T2, typename IndexSet>
inline
typename DividesType<SparseNumberStruct<IndexSet>,T2>::supertype
operator / (const SparseNumberStruct<IndexSet>& a, const T2& b)
{
  typedef typename DividesType<SparseNumberStruct<IndexSet>,T2>::supertype type;

  type returnval;
  typename IndexSet::ForEach()
    (BinaryFunctor<DividesSubfunctor, IndexSet, ConstantDataSet<T2>, typename type::index_set>
      (DividesSubfunctor(), a.raw_data(), ConstantDataSet<T2>(b), returnval.raw_data()));
  return returnval;
}


// FIXME: Set::rebind<bool> won't work as desired for tensors

#define SparseNumberStruct_operator_binary(opname, functorname) \
\
struct functorname##_Subfunctor { \
  template <typename T> \
  bool operator()(T& x, T& y) const { return std::functorname<T>(x,y); } \
}; \
\
template <typename IndexSet, typename IndexSet2> \
inline \
SparseNumberStruct<typename IndexSet::template Union<IndexSet2>::type::template rebind<bool>::other> \
operator opname (const SparseNumberStruct<IndexSet>& a, const SparseNumberStruct<IndexSet2>& b) \
{ \
  typedef typename IndexSet::template Union<IndexSet2>::type IS; \
  typedef typename IS::template rebind<bool>::other IB; \
  SparseNumberStruct<IB> returnval; \
 \
  typename IndexSet::template Intersection<IndexSet2>::type::ForEach() \
    (BinaryFunctor<functorname##_Subfunctor,IndexSet,IndexSet2,IB> \
      (functorname##_Subfunctor(), a.raw_data(), b.raw_data(), returnval.raw_data())); \
  typename IndexSet::template Difference<IndexSet2>::type::ForEach() \
    (BinaryFunctor<functorname##_Subfunctor,IndexSet,ConstantDataSet<int>,IB> \
      (functorname##_Subfunctor(), a.raw_data(), ConstantDataSet<int>(0), returnval.raw_data())); \
  typename IndexSet2::template Difference<IndexSet>::type::ForEach() \
    (BinaryFunctor<functorname##_Subfunctor,ConstantDataSet<int>,IndexSet2,IB> \
      (functorname##_Subfunctor(), ConstantDataSet<int>(0), b.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
} \
template <typename T2, typename IndexSet> \
inline \
typename SparseNumberStruct<IndexSet>::template rebind<bool>::other \
operator opname (const SparseNumberStruct<IndexSet>& a, const T2& b) \
{ \
  typedef typename SparseNumberStruct<IndexSet>::template rebind<bool>::other IB; \
  SparseNumberStruct<IB> returnval; \
 \
  typename IndexSet::ForEach() \
    (BinaryFunctor<functorname##_Subfunctor,IndexSet,ConstantDataSet<T2>,IB> \
      (functorname##_Subfunctor(), a.raw_data(), ConstantDataSet<T2>(b), returnval.raw_data())); \
 \
  return returnval; \
} \
template <typename T, typename IndexSet> \
inline \
typename SparseNumberStruct<IndexSet>::template rebind<bool>::other \
operator opname (const T& a, const SparseNumberStruct<IndexSet>& b) \
{ \
  typedef typename SparseNumberStruct<IndexSet>::template rebind<bool>::other IB; \
  SparseNumberStruct<IB> returnval; \
 \
  typename IndexSet::ForEach() \
    (BinaryFunctor<functorname##_Subfunctor,ConstantDataSet<T>,IndexSet,IB> \
      (functorname##_Subfunctor(), ConstantDataSet<T>(a), b.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
}

// NOTE: unary functions for which 0-op-0 is true are undefined compile-time
// errors, because there's no efficient way to have them make sense in
// the sparse context.

SparseNumberStruct_operator_binary(<, less)
// SparseNumberStruct_operator_binary(<=)
SparseNumberStruct_operator_binary(>, greater)
// SparseNumberStruct_operator_binary(>=)
// SparseNumberStruct_operator_binary(==)
SparseNumberStruct_operator_binary(!=, not_equal_to)

// FIXME - make && an intersection rather than a union for efficiency
SparseNumberStruct_operator_binary(&&, logical_and)
SparseNumberStruct_operator_binary(||, logical_or)

// Making this a local struct seems to fail??
template <typename IndexSet>
struct SparseNumberStructOutputFunctor {
  SparseNumberStructOutputFunctor(std::ostream& o, const IndexSet &in) : _out(o), _in(in) {}

  template <typename ValueType>
  inline void operator()() const {
    _out << ", (" << ValueType::value << ',' <<
            _in.template data<ValueType>() << ')';
  }

private:
  std::ostream& _out;
  const IndexSet& _in;
};


template <typename IndexSet>
inline
std::ostream&      
operator<< (std::ostream& output, const SparseNumberStruct<IndexSet>& a)
{
  // Enclose the entire output in braces
  output << '{';

  // Output the first value from a non-empty set
  // All values are given as ordered (index, value) pairs
  if (IndexSet::size)
    output << '(' << IndexSet::head_type::value << ',' <<
              a.raw_data().template data<typename IndexSet::head_type>() << ')';

  // Output the comma-separated subsequent values from a non-singleton
  // set
  if (IndexSet::size > 1)
    typename IndexSet::tail_set::ForEach()
      (SparseNumberStructOutputFunctor<typename IndexSet::tail_set>(output, a.raw_data().tail));

  output << '}';
  return output;
}


// CompareTypes, RawType, ValueType specializations

#define SparseNumberStruct_comparisons(templatename, settype) \
template<typename IndexSet, bool reverseorder> \
struct templatename<SparseNumberStruct<IndexSet>, \
                    SparseNumberStruct<IndexSet>, reverseorder> { \
  typedef SparseNumberStruct<IndexSet> supertype; \
}; \
 \
template<typename IndexSet, typename IndexSet2, bool reverseorder> \
struct templatename<SparseNumberStruct<IndexSet>, \
                    SparseNumberStruct<IndexSet2>, reverseorder> { \
  typedef SparseNumberStruct<typename IndexSet::template settype<IndexSet2>::type> supertype; \
}; \
 \
template<typename T2, typename IndexSet, bool reverseorder> \
struct templatename<SparseNumberStruct<IndexSet>, T2, reverseorder, \
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> { \
  typedef SparseNumberStruct<typename Symmetric##templatename< \
    IndexSet, T2, reverseorder \
  >::supertype> supertype; \
}; \
 \
template<typename IndexSet, bool reverseorder> \
struct templatename<SparseNumberStruct<IndexSet>, MetaPhysicL::NullType, reverseorder> { \
  typedef SparseNumberStruct<IndexSet> supertype; \
}; \
 \
template<typename HeadType, typename TailSet, typename IndexSet, typename Comparison> \
struct templatename<SparseNumberStruct<IndexSet>, MetaPhysicL::Container<HeadType,TailSet,Comparison> > { \
  typedef SparseNumberStruct< \
    typename Symmetric##templatename<IndexSet, MetaPhysicL::Container<HeadType,TailSet,Comparison> >::supertype \
  > supertype; \
}

SparseNumberStruct_comparisons(CompareTypes, Union);
SparseNumberStruct_comparisons(PlusType, Union);
SparseNumberStruct_comparisons(MinusType, Union);
SparseNumberStruct_comparisons(MultipliesType, Intersection);
SparseNumberStruct_comparisons(DividesType, First);
SparseNumberStruct_comparisons(AndType, Intersection);
SparseNumberStruct_comparisons(OrType, Union);



template <typename CookedSet>
struct RawSet
{
  typedef
    MetaPhysicL::Container<
      typename CookedSet::head_type::template rebind<
        typename RawType<typename CookedSet::head_type::data_type>::value_type>::other,
      typename RawSet<typename CookedSet::tail_set>::type,
      typename CookedSet::comparison
    > type;
};


template <typename NullHeadType>
struct RawSet<MetaPhysicL::NullContainer<NullHeadType> >
{
  typedef MetaPhysicL::NullContainer<NullHeadType> type;
};


struct RawTypeSubfunctor
{
  template <typename T>
  typename RawType<T>::value_type
  operator()(T& x) const { return RawType<T>::value(x); }
};


template <typename IndexSet>
struct RawType<SparseNumberStruct<IndexSet> >
{
  typedef typename RawSet<IndexSet>::type RawIS;
  typedef SparseNumberStruct<RawIS> value_type;

  static value_type value(const SparseNumberStruct<IndexSet>& a)
    {
      value_type returnval;
      returnval.for_each_datum
        (UnaryFunctor<RawTypeSubfunctor,IndexSet,RawIS>
          (RawTypeSubfunctor(), a.raw_data(), returnval.raw_data()));
      return returnval;
    }
};

template <typename IndexSet>
struct ValueType<SparseNumberStruct<IndexSet> >
{
  typedef typename SparseNumberStruct<IndexSet>::value_type type;
};


} // namespace MetaPhysicL


namespace std {

using MetaPhysicL::SparseNumberStruct;
using MetaPhysicL::BinaryFunctor;
using MetaPhysicL::UnaryFunctor;
using MetaPhysicL::ConstantDataSet;

#define SparseNumberStruct_std_unary(funcname) \
\
struct funcname##_Subfunctor { \
  template <typename T> \
  T operator()(T& x) const { return std::funcname(x); } \
}; \
\
template <typename IndexSet> \
inline \
SparseNumberStruct<IndexSet> \
funcname (SparseNumberStruct<IndexSet> a) \
{ \
  a.for_each_datum \
    (UnaryFunctor<funcname##_Subfunctor,IndexSet,IndexSet> \
      (funcname##_Subfunctor(), a.raw_data(), a.raw_data())); \
 \
  return a; \
}


#define SparseNumberStruct_std_binary(funcname) \
\
struct funcname##_Subfunctor { \
  template <typename T1, typename T2> \
  typename CompareTypes<T1,T2>::supertype \
  operator()(T1& x, T2& y) const { return std::funcname(x,y); } \
}; \
\
template <typename IndexSet, typename IndexSet2> \
inline \
SparseNumberStruct<typename IndexSet2::template Intersection<IndexSet>::template Union<IndexSet>::type> \
funcname (const SparseNumberStruct<IndexSet>& a, const SparseNumberStruct<IndexSet2>& b) \
{ \
  /* Intersection/Union hackery here handles data_type upgrading */ \
  typedef typename \
    IndexSet2::template Intersection<IndexSet>::template Union<IndexSet>::type IS; \
  SparseNumberStruct<IS> returnval; \
 \
  typename IndexSet::ForEach() \
    (BinaryFunctor<funcname##_Subfunctor,IndexSet,IndexSet2,IS> \
      (funcname##_Subfunctor(), a.raw_data(), b.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
} \
\
template <typename IndexSet, typename T2> \
inline \
typename CompareTypes<SparseNumberStruct<IndexSet>,T2>::supertype \
funcname (const SparseNumberStruct<IndexSet>& a, const T2& b) \
{ \
  typedef typename CompareTypes<SparseNumberStruct<IndexSet>,T2>::supertype type; \
  type returnval; \
 \
  typename IndexSet::ForEach() \
    (BinaryFunctor<funcname##_Subfunctor,IndexSet,ConstantDataSet<T2>,typename type::index_set> \
      (funcname##_Subfunctor(), a.raw_data(), ConstantDataSet<T2>(b), returnval.raw_data())); \
 \
  return returnval; \
} \
 \
template <typename T, typename IndexSet> \
inline \
typename CompareTypes<SparseNumberStruct<IndexSet>,T,true>::supertype \
funcname (const T& a, const SparseNumberStruct<IndexSet>& b) \
{ \
  typedef typename CompareTypes<SparseNumberStruct<IndexSet>,T,true>::supertype type; \
  type returnval; \
 \
  typename IndexSet::ForEach() \
    (BinaryFunctor<funcname##_Subfunctor,ConstantDataSet<T>,IndexSet,typename type::index_set> \
      (funcname##_Subfunctor(), ConstantDataSet<T>(a), b.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
}


#define SparseNumberStruct_std_binary_union(funcname) \
\
struct funcname##_Subfunctor { \
  template <typename T1, typename T2> \
  typename CompareTypes<T1,T2>::supertype \
  operator()(const T1& x, const T2& y) const { \
    typedef typename CompareTypes<T1,T2>::supertype TS; \
    return std::funcname(TS(x),TS(y)); \
  } \
}; \
 \
template <typename IndexSet, typename IndexSet2> \
inline \
SparseNumberStruct<typename IndexSet::template Union<IndexSet2>::type> \
funcname (const SparseNumberStruct<IndexSet>& a, const SparseNumberStruct<IndexSet2>& b) \
{ \
  typedef typename IndexSet::template Union<IndexSet2>::type IS; \
  SparseNumberStruct<IS> returnval; \
 \
  typename IndexSet::template Intersection<IndexSet2>::type::ForEach() \
    (BinaryFunctor<funcname##_Subfunctor,IndexSet,IndexSet2,IS> \
      (funcname##_Subfunctor(), a.raw_data(), b.raw_data(), returnval.raw_data())); \
  typename IndexSet::template Difference<IndexSet2>::type::ForEach() \
    (BinaryFunctor<funcname##_Subfunctor,IndexSet,ConstantDataSet<int>,IS> \
      (funcname##_Subfunctor(), a.raw_data(), ConstantDataSet<int>(0), returnval.raw_data())); \
  typename IndexSet2::template Difference<IndexSet>::type::ForEach() \
    (BinaryFunctor<funcname##_Subfunctor,ConstantDataSet<int>,IndexSet2,IS> \
      (funcname##_Subfunctor(), ConstantDataSet<int>(0), b.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
} \
 \
template <typename IndexSet> \
inline \
SparseNumberStruct<IndexSet> \
funcname (const SparseNumberStruct<IndexSet>& a, const SparseNumberStruct<IndexSet> b) \
{ \
  SparseNumberStruct<IndexSet> returnval; \
 \
  typename IndexSet::ForEach() \
    (BinaryFunctor<funcname##_Subfunctor,IndexSet,IndexSet,IndexSet> \
      (funcname##_Subfunctor(), a.raw_data(), b.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
} \
 \
template <typename T2, typename IndexSet> \
inline \
typename CompareTypes<SparseNumberStruct<IndexSet>,T2>::supertype \
funcname (const SparseNumberStruct<IndexSet>& a, const T2& b) \
{ \
  typedef typename CompareTypes<SparseNumberStruct<IndexSet>,T2>::supertype type; \
  type returnval; \
 \
  typename IndexSet::ForEach() \
    (BinaryFunctor<funcname##_Subfunctor,IndexSet,ConstantDataSet<T2>,typename type::index_set> \
      (funcname##_Subfunctor(), a.raw_data(), ConstantDataSet<T2>(b), returnval.raw_data())); \
 \
  return returnval; \
} \
 \
template <typename T, typename IndexSet> \
inline \
typename CompareTypes<SparseNumberStruct<IndexSet>,T,true>::supertype \
funcname (const T& a, const SparseNumberStruct<IndexSet>& b) \
{ \
  typedef typename CompareTypes<SparseNumberStruct<IndexSet>,T,true>::supertype type; \
  type returnval; \
 \
  typename IndexSet::ForEach() \
    (BinaryFunctor<funcname##_Subfunctor,ConstantDataSet<T>,IndexSet,typename type::index_set> \
      (funcname##_Subfunctor(), ConstantDataSet<T>(a), b.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
}



// NOTE: unary functions for which f(0) != 0 are undefined compile-time
// errors, because there's no efficient way to have them make sense in
// the sparse context.

SparseNumberStruct_std_binary(pow)
// SparseNumberStruct_std_unary(exp)
// SparseNumberStruct_std_unary(log)
// SparseNumberStruct_std_unary(log10)
SparseNumberStruct_std_unary(sin)
// SparseNumberStruct_std_unary(cos)
SparseNumberStruct_std_unary(tan)
// SparseNumberStruct_std_unary(asin)
// SparseNumberStruct_std_unary(acos)
SparseNumberStruct_std_unary(atan)
SparseNumberStruct_std_binary_union(atan2)
SparseNumberStruct_std_unary(sinh)
// SparseNumberStruct_std_unary(cosh)
SparseNumberStruct_std_unary(tanh)
SparseNumberStruct_std_unary(sqrt)
SparseNumberStruct_std_unary(abs)
SparseNumberStruct_std_unary(fabs)
SparseNumberStruct_std_binary_union(max)
SparseNumberStruct_std_binary_union(min)
SparseNumberStruct_std_unary(ceil)
SparseNumberStruct_std_unary(floor)
SparseNumberStruct_std_binary(fmod) // dangerous unless y is dense


// Defining numeric_limits for heterogenous containers is pretty much
// impossible
/*
template <typename IndexSet>
class numeric_limits<SparseNumberStruct<IndexSet> > : 
  public raw_numeric_limits<SparseNumberStruct<IndexSet>, IDunno> {};
*/

} // namespace std


#endif // METAPHYSICL_SPARSENUMBERSTRUCT_H
