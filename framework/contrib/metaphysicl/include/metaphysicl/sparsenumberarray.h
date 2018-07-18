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


#ifndef METAPHYSICL_SPARSENUMBERARRAY_H
#define METAPHYSICL_SPARSENUMBERARRAY_H

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <ostream>

#include "metaphysicl/compare_types.h"
#include "metaphysicl/ct_set.h"
#include "metaphysicl/metaprogramming.h" // for call_traits
#include "metaphysicl/metaphysicl_asserts.h"
#include "metaphysicl/raw_type.h"
#include "metaphysicl/sparsenumberutils.h"
#include "metaphysicl/testable.h"

namespace MetaPhysicL {

// Forward declarations
template <typename T, typename IndexSet>
class SparseNumberArray;

template<typename IndexSet1, typename IndexSet2, typename S, typename T, bool reverseorder>
struct DotType<SparseNumberArray<S,IndexSet1>,
               SparseNumberArray<T,IndexSet2>, reverseorder> {
  typedef
    SparseNumberArray<typename DotType<S,T,reverseorder>::supertype,
                      typename IndexSet1::template
                        Intersection<IndexSet2>::type> supertype;
};

template<typename IndexSet1, typename IndexSet2, typename S, typename T, bool reverseorder>
struct OuterProductType<SparseNumberArray<S,IndexSet1>,
                        SparseNumberArray<T,IndexSet2>, reverseorder> {
  typedef 
  SparseNumberArray<
    typename OuterProductType<S,T,reverseorder>::supertype,
    typename IndexSet1::template Intersection<IndexSet2>::type> supertype;
};

template<typename S, typename IndexSet>
struct SumType<SparseNumberArray<S,IndexSet> > {
  typedef SparseNumberArray<typename SumType<S>::supertype, IndexSet> supertype;
};


template <typename T, typename IndexSet>
class SparseNumberArray : public safe_bool<SparseNumberArray<T, IndexSet> >
{
public:
  typedef T value_type;

  template <unsigned int i>
  struct entry_type {
    typedef value_type type;
  };

  typedef IndexSet index_set;

  typedef typename IndexSet::head_type index_type;

  typedef typename index_type::value_type index_value_type;

  template <typename T2>
  struct rebind {
    typedef SparseNumberArray<T2, IndexSet> other;
  };

  static const size_t index_size = IndexSet::size;

  std::size_t size() const
    { return IndexSet::size; }

  SparseNumberArray() {}

  SparseNumberArray(const T& val) {
    // This makes no sense unless val is 0!
#ifndef NDEBUG
    if (val)
      throw std::domain_error("Cannot initialize SparseNumberArray with non-zero scalar");
#endif
    std::fill(raw_data(), raw_data()+size(), val);
  }

  template <typename T2>
  SparseNumberArray(const T2& val) {
    // This makes no sense unless val is 0!
#ifndef NDEBUG
    if (val)
      throw std::domain_error("Cannot initialize SparseNumberArray with non-zero scalar");
#endif
    std::fill(raw_data(), raw_data()+size(), T(val));
  }

  template <typename T2>
  SparseNumberArray(SparseNumberArray<T2, IndexSet> src)
    { std::copy(src.raw_data(), src.raw_data()+size(), raw_data()); }

  template <bool, typename ValueType, typename IndexSet2>
  struct SubCopyFunctor {
    template<typename Tin, typename Tout>
    inline static void apply(const Tin *in, Tout* out) {
      const unsigned int
        indexin  = IndexSet2::template IndexOf<ValueType>::index,
        indexout = IndexSet::template IndexOf<ValueType>::index;
      out[indexout] = in[indexin];
    }
  };

  template <typename ValueType, typename IndexSet2>
  struct SubCopyFunctor<false, ValueType, IndexSet2> {
    template<typename Tin, typename Tout>
    inline static void apply(const Tin *, Tout* out) {
      const unsigned int
        indexout = IndexSet::template IndexOf<ValueType>::index;
      out[indexout] = 0;
    }
  };

  template <typename IndexSet2, typename Tin>
  struct CopyFunctor {
    CopyFunctor(const Tin* in, T* out) : _datain(in), _dataout(out) {}

    template <typename ValueType>
    inline void operator()() const {
      SubCopyFunctor<IndexSet2::template Contains<ValueType>::value,
               ValueType, IndexSet2>::apply(_datain, _dataout);
    }

  private:
    const Tin* _datain;
    T* _dataout;
  };

  template <typename SubFunctor, typename IndexSet2, typename Tin>
  struct OpEqualsFunctor {
    OpEqualsFunctor(SubFunctor f, const Tin* in, T* out) :
      _subfunctor(f), _datain(in), _dataout(out) {}

    template <typename ValueType>
    inline void operator()() const {
      const unsigned int
        indexin  = IndexSet2::template IndexOf<ValueType>::index,
        indexout = IndexSet::template IndexOf<ValueType>::index;
      _dataout[indexout] = _subfunctor(_dataout[indexout], _datain[indexin]);
    }

  private:
    SubFunctor _subfunctor;
    const Tin* _datain;
    T* _dataout;
  };

  template <typename SubFunctor, typename Tout>
  struct UnaryIteratedFunctor {
    UnaryIteratedFunctor(SubFunctor f, const T* in, Tout& out) :
      _subfunctor(f), _in(in), _out(out) {}

    template <typename ValueType>
    inline void operator()() const {
      const unsigned int
        indexin = IndexSet::template IndexOf<ValueType>::index;
      _subfunctor(_out, _in[indexin]);
    }

  private:
    SubFunctor _subfunctor;
    const T* _in;
    Tout& _out;
  };


  template <typename SubFunctor, typename IndexSet2, typename T2, typename Tout>
  struct BinaryIteratedFunctor {
    BinaryIteratedFunctor(SubFunctor f, const T* in1, const T2* in2, Tout& out) :
      _subfunctor(f), _datain1(in1), _datain2(in2), _out(out) {}

    template <typename ValueType>
    inline void operator()() const {
      const unsigned int
        indexin1 = IndexSet::template IndexOf<ValueType>::index,
        indexin2 = IndexSet2::template IndexOf<ValueType>::index;
      _subfunctor(_datain1[indexin1], _datain2[indexin2], _out);
    }

  private:
    SubFunctor _subfunctor;
    const T* _datain1;
    const T2* _datain2;
    Tout& _out;
  };

  template <typename T1, typename T2>
  struct AccumulateDot {
    inline void operator()(const T1& in1, const T2& in2,
                           typename SymmetricMultipliesType<T1,T2>::supertype& out) const
      { out += in1 * in2; }
  };

  struct SetZeroFunctor {
    SetZeroFunctor(T* out) : _dataout(out) {}

    template <typename ValueType>
    inline void operator()() {
      const unsigned int
        indexout = IndexSet::template IndexOf<ValueType>::index;
      _dataout[indexout] = 0;
    }

  private:
    T* _dataout;
  };

  // We can have an implicit constructor that gives a compile-time
  // error for IndexSet2 which is not a subset of IndexSet
  template <typename T2, typename IndexSet2>
  SparseNumberArray(SparseNumberArray<T2, IndexSet2> src) {
    typename IndexSet::ForEach()
      (CopyFunctor<IndexSet2,T2>(src.raw_data(), raw_data()));
    ctassert<IndexSet2::template Difference<IndexSet>::type::size == 0>::apply();
  }

  // We have a dangerous, explicit named constructor that silently
  // drops data for IndexSet2 which is not a subset of IndexSet
  template <typename T2, typename IndexSet2>
  static SparseNumberArray slice(SparseNumberArray<T2, IndexSet2> src) {
    SparseNumberArray returnval;
    typename IndexSet::template Intersection<IndexSet2>::type::ForEach()
      (CopyFunctor<IndexSet2,T2>(src.raw_data(), returnval.raw_data()));
    typename IndexSet::template Difference<IndexSet2>::type::ForEach()
      (SetZeroFunctor(returnval.raw_data()));
    return returnval;
  }

#if  __cplusplus >= 201103L
  std::array<T,index_size>& raw_data_array()
    { return _data; }

  const std::array<T,index_size>& raw_data_array() const
    { return _data; }
#endif

  T* raw_data()
    { return index_size?&_data[0]:NULL; }

  const T* raw_data() const
    { return index_size?&_data[0]:NULL; }

  T& raw_at(unsigned int i)
    { return _data[i]; }

  const T& raw_at(unsigned int i) const
    { return _data[i]; }

  T& operator[](index_value_type i)
    { return _data[IndexSet::runtime_index_of(i)]; }

  const T& operator[](index_value_type i) const
    { return _data[IndexSet::runtime_index_of(i)]; }

  template <unsigned int i>
  typename entry_type<i>::type& get() {
    return _data[IndexSet::template IndexOf<MetaPhysicL::UnsignedIntType<i> >::index];
  }

  template <unsigned int i>
  const typename entry_type<i>::type& get() const {
    return _data[IndexSet::template IndexOf<MetaPhysicL::UnsignedIntType<i> >::index];
  }

  template <unsigned int i>
  typename entry_type<i>::type& insert() {
    return _data[IndexSet::template IndexOf<MetaPhysicL::UnsignedIntType<i> >::index];
  }

  template <unsigned int i, typename T2>
  void set(const T2& val) {
    _data[IndexSet::template IndexOf<MetaPhysicL::UnsignedIntType<i> >::index] = val;
  }

  bool boolean_test() const {
    for (unsigned int i=0; i != index_size; ++i)
      if (_data[i])
        return true;
    return false;
  }

  SparseNumberArray<T,IndexSet> operator- () const {
    SparseNumberArray<T,IndexSet> returnval;
    for (unsigned int i=0; i != index_size; ++i) returnval.raw_at(i) = -_data[i];
    return returnval;
  }

  // Not defineable since !0 != 0
  // SparseNumberArray<T,IndexSet> operator! () const;

  template <typename T2, typename IndexSet2>
  SparseNumberArray<T,IndexSet>&
    operator+= (const SparseNumberArray<T2,IndexSet2>& a) { 
    typename IndexSet2::ForEach()
      (OpEqualsFunctor<std::plus<T>, IndexSet2, T2>
        (std::plus<T>(), a.raw_data(), raw_data()));
    ctassert<IndexSet2::template Difference<IndexSet>::type::size == 0>::apply();
    return *this;
  }

  template <typename T2, typename IndexSet2>
  SparseNumberArray<T,IndexSet>&
    operator-= (const SparseNumberArray<T2,IndexSet2>& a) { 
    typename IndexSet2::ForEach()
      (OpEqualsFunctor<std::minus<T>, IndexSet2, T2>
        (std::minus<T>(), a.raw_data(), raw_data()));
    ctassert<IndexSet2::template Difference<IndexSet>::type::size == 0>::apply();
    return *this;
  }

  template <typename T2, typename IndexSet2>
  SparseNumberArray<T,IndexSet>&
    operator*= (const SparseNumberArray<T2,IndexSet2>& a) { 
    typename IndexSet::template Intersection<IndexSet2>::type::ForEach()
      (OpEqualsFunctor<std::multiplies<T>, IndexSet2, T2>
        (std::multiplies<T>(), a.raw_data(), raw_data()));
    typename IndexSet::template Difference<IndexSet2>::type::ForEach()
      (SetZeroFunctor(raw_data()));
    return *this;
  }

  template <typename T2, typename IndexSet2>
  SparseNumberArray<T,IndexSet>&
    operator/= (const SparseNumberArray<T2,IndexSet2>& a) { 
    typename IndexSet::ForEach()
      (OpEqualsFunctor<std::divides<T>, IndexSet2, T2>
        (std::divides<T>(), a.raw_data(), raw_data()));
    ctassert<IndexSet::template Difference<IndexSet2>::type::size == 0>::apply();
    return *this;
  }

  template <typename T2>
  SparseNumberArray<T,IndexSet>& operator*= (const T2& a)
    { for (unsigned int i=0; i != index_size; ++i) _data[i] *= a; return *this; }

  template <typename T2>
  SparseNumberArray<T,IndexSet>& operator/= (const T2& a)
    { for (unsigned int i=0; i != index_size; ++i) _data[i] /= a; return *this; }

  template <typename T2, typename IndexSet2>
  SparseNumberArray<typename DotType<T,T2>::supertype,
                    typename IndexSet::template
                      Intersection<IndexSet2>::type>
  dot (const SparseNumberArray<T2,IndexSet2>& a) const
  {
    typedef typename DotType<T,T2>::supertype TS;
    typedef typename IndexSet::template Intersection<IndexSet2>::type IndexSetS;

    SparseNumberArray<TS, IndexSetS> returnval;

    metaphysicl_not_implemented();
//    IndexSetS::ForEach()
//      (BinaryVectorFunctor<std::binary_function<TS,TS,TS>, IndexSet,
//       IndexSet2,IndexSetS,T,T2,TS>
//        (this->raw_data(), a.raw_data(), returnval.raw_data(),
//         std::multiplies<TS>));

    return returnval;
  }

  template <typename T2, typename IndexSet2>
  SparseNumberArray<
    typename OuterProductType<T,T2>::supertype,
    typename IndexSet::template Intersection<IndexSet2>::type>
  outerproduct (const SparseNumberArray<T2, IndexSet2>& a) const
  {
    typedef typename OuterProductType<T,T2>::supertype TS;
    typedef typename IndexSet::template Intersection<IndexSet2>::type IndexSetS;
    SparseNumberArray<TS, IndexSetS> returnval;

    metaphysicl_not_implemented();
//    IndexSetS::ForEach()
//      (BinaryVectorFunctor<std::pointer_to_binary_function<T,T2,TS>,
//       IndexSet, IndexSet2, IndexSetS, T, T2, TS>
//        (this->raw_data(), a.raw_data(), returnval.raw_data(),
//         MetaPhysicL::binary_ptr_fun(outerproduct));

    return returnval;
  }

private:
#if  __cplusplus >= 201103L
  std::array<T,index_size> _data;
#else
  T _data[index_size];
#endif
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



template <unsigned int N,
          unsigned int index1=0, typename Data1=void,
          unsigned int index2=0, typename Data2=void,
          unsigned int index3=0, typename Data3=void,
          unsigned int index4=0, typename Data4=void,
          unsigned int index5=0, typename Data5=void,
          unsigned int index6=0, typename Data6=void,
          unsigned int index7=0, typename Data7=void,
          unsigned int index8=0, typename Data8=void>
struct SparseNumberArrayOf
{
  template <unsigned int i, typename Data>
  struct UIntOrNullType {
    typedef MetaPhysicL::UnsignedIntType<i> type;
  };

  template <unsigned int i>
  struct UIntOrNullType<i, void> {
    typedef MetaPhysicL::NullContainer<MetaPhysicL::UnsignedIntType<0> > type;
  };

  typedef
  typename SymmetricCompareTypes<Data1,
    typename SymmetricCompareTypes<Data2,
      typename SymmetricCompareTypes<Data3,
        typename SymmetricCompareTypes<Data4,
          typename SymmetricCompareTypes<Data5,
            typename SymmetricCompareTypes<Data6,
              typename SymmetricCompareTypes<Data7,Data8>::supertype
            >::supertype
          >::supertype
        >::supertype
      >::supertype
    >::supertype
  >::supertype supertype;

  typedef SparseNumberArray<
    supertype,
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



template <std::size_t N, unsigned int index, typename T>
struct SparseNumberArrayUnitVector
{
  typedef MetaPhysicL::Container<
    MetaPhysicL::UnsignedIntType<index>,
    MetaPhysicL::NullContainer<MetaPhysicL::UnsignedIntType<0> >
  > IndexSet;

  typedef SparseNumberArray<T, IndexSet> type;

  static type value() {
    type returnval;
    returnval.raw_at(0) = 1;
    return returnval;
  }
};


template <std::size_t N, typename T>
struct SparseNumberArrayFullVector
{
  typedef MetaPhysicL::Container<
    MetaPhysicL::UnsignedIntType<N-1>,
    typename SparseNumberArrayFullVector<N-1,T>::IndexSet
  > IndexSet;

  typedef SparseNumberArray<T,IndexSet> type;

  static type value() {
    type returnval;
    for (unsigned int i=0; i != N; ++i)
      returnval[i] = 1;
    return returnval;
  }
};


template <typename T>
struct SparseNumberArrayFullVector<0,T>
{
  typedef MetaPhysicL::NullContainer<MetaPhysicL::UnsignedIntType<0> > IndexSet;

  typedef SparseNumberArray<T,IndexSet> type;

  static type value() {
    type returnval;
    return returnval;
  }
};




template <typename T, typename IndexSet, typename IndexSet2>
inline
SparseNumberArray<SparseNumberArray<T, IndexSet>, IndexSet2>
transpose(const SparseNumberArray<SparseNumberArray<T, IndexSet2>, IndexSet>& a)
{
  const unsigned int size  = IndexSet::size;
  const unsigned int size2 = IndexSet2::size;

  SparseNumberArray<SparseNumberArray<T, IndexSet>, IndexSet2> returnval;

  for (unsigned int i=0; i != size; ++i)
    for (unsigned int j=0; j != size2; ++j)
      returnval.raw_at(j).raw_at(i) = a.raw_at(i).raw_at(j);

  return returnval;
}


template <typename T, typename IndexSet>
SparseNumberArray<typename SumType<T>::supertype, IndexSet>
sum (const SparseNumberArray<T, IndexSet> &a)
{
  SparseNumberArray<typename SumType<T>::supertype, IndexSet>
    returnval = 0;

  for (unsigned int i=0; i != IndexSet::size; ++i)
    returnval.raw_at(i) = a.raw_at(i).sum();

  return returnval;
}



#define SparseNumberArray_op_ab(opname, atype, btype, functorname) \
template <typename T, typename T2, typename IndexSet, typename IndexSet2> \
inline \
typename Symmetric##functorname##Type<atype,btype>::supertype \
operator opname (const atype& a, const btype& b) \
{ \
  typedef typename Symmetric##functorname##Type<atype,btype>::supertype type; \
  type returnval = type::slice(a); \
  returnval opname##= b; \
  return returnval; \
}

#define SparseNumberArray_op(opname, functorname) \
SparseNumberArray_op_ab(opname, SparseNumberArray<T MacroComma IndexSet>, SparseNumberArray<T2 MacroComma IndexSet2>, functorname)

SparseNumberArray_op(+, Plus)       // Union)
SparseNumberArray_op(-, Minus)      // Union)
SparseNumberArray_op(*, Multiplies) // Intersection)
SparseNumberArray_op(/, Divides)    // First)

// Let's also allow scalar times vector.
// Scalar plus vector, etc. remain undefined in the sparse context.

template <typename T, typename T2, typename IndexSet>
inline
typename MultipliesType<SparseNumberArray<T2,IndexSet>,T,true>::supertype
operator * (const T& a, const SparseNumberArray<T2,IndexSet>& b)
{
  const unsigned int size = IndexSet::size;

  typename MultipliesType<SparseNumberArray<T2,IndexSet>,T,true>::supertype returnval;
  for (unsigned int i=0; i != size; ++i)
    returnval.raw_at(i) = a * b.raw_at(i);
  return returnval;
}

template <typename T, typename T2, typename IndexSet>
inline
typename MultipliesType<SparseNumberArray<T,IndexSet>,T2>::supertype
operator * (const SparseNumberArray<T,IndexSet>& a, const T2& b)
{
  const unsigned int size = IndexSet::size;

  typename MultipliesType<SparseNumberArray<T,IndexSet>,T2>::supertype returnval;
  for (unsigned int i=0; i != size; ++i)
    returnval.raw_at(i) = a.raw_at(i) * b;
  return returnval;
}

template <typename T, typename T2, typename IndexSet>
inline
typename DividesType<SparseNumberArray<T,IndexSet>,T2>::supertype
operator / (const SparseNumberArray<T,IndexSet>& a, const T2& b)
{
  const unsigned int size = IndexSet::size;

  typename DividesType<SparseNumberArray<T,IndexSet>,T2>::supertype returnval;
  for (unsigned int i=0; i != size; ++i)
    returnval.raw_at(i) = a.raw_at(i) / b;
  return returnval;
}


#define SparseNumberArray_operator_binary(opname, functorname) \
template <typename T, typename T2, typename IndexSet, typename IndexSet2> \
inline \
SparseNumberArray<bool, typename IndexSet::template Union<IndexSet2>::type> \
operator opname (const SparseNumberArray<T,IndexSet>& a, const SparseNumberArray<T2,IndexSet2>& b) \
{ \
  typedef typename SymmetricCompareTypes<T,T2>::supertype TS; \
  typedef typename IndexSet::template Union<IndexSet2>::type IndexSetS; \
  SparseNumberArray<bool, IndexSetS> returnval; \
 \
  typename IndexSet::template Intersection<IndexSet2>::type::ForEach() \
    (BinaryVectorFunctor<std::functorname<TS>,IndexSet,IndexSet2,IndexSetS,T,T2,bool> \
      (std::functorname<TS>(), a.raw_data(), b.raw_data(), returnval.raw_data())); \
  typename IndexSet::template Difference<IndexSet2>::type::ForEach() \
    (UnaryVectorFunctor<std::unary_function<T,bool>,IndexSet,IndexSetS,T,bool> \
      (std::bind2nd(std::functorname<T>(),TS(0)), a.raw_data(), returnval.raw_data())); \
  typename IndexSet2::template Difference<IndexSet>::type::ForEach() \
    (UnaryVectorFunctor<std::unary_function<T,bool>,IndexSet2,IndexSetS,T2,bool> \
      (std::bind1st(std::functorname<T2>(),T2(0)), b.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
} \
template <typename T, typename T2, typename IndexSet> \
inline \
SparseNumberArray<bool, IndexSet> \
operator opname (const SparseNumberArray<T, IndexSet>& a, const T2& b) \
{ \
  SparseNumberArray<bool, IndexSet> returnval; \
 \
  for (unsigned int i=0; i != IndexSet::size; ++i) \
    returnval.raw_at(i) = (a.raw_at(i) opname b); \
 \
  return returnval; \
} \
template <typename T, typename T2, typename IndexSet> \
inline \
SparseNumberArray<bool, IndexSet> \
operator opname (const T& a, const SparseNumberArray<T2,IndexSet>& b) \
{ \
  SparseNumberArray<bool, IndexSet> returnval; \
 \
  for (unsigned int i=0; i != IndexSet::size; ++i) \
    returnval.raw_at(i) = (a opname b.raw_at(i)); \
 \
  return returnval; \
}

// NOTE: unary functions for which 0-op-0 is true are undefined compile-time
// errors, because there's no efficient way to have them make sense in
// the sparse context.

SparseNumberArray_operator_binary(<, less)
// SparseNumberArray_operator_binary(<=)
SparseNumberArray_operator_binary(>, greater)
// SparseNumberArray_operator_binary(>=)
// SparseNumberArray_operator_binary(==)
SparseNumberArray_operator_binary(!=, not_equal_to)

// TODO - make && an intersection rather than a union for efficiency
SparseNumberArray_operator_binary(&&, logical_and)
SparseNumberArray_operator_binary(||, logical_or)

// Making this a local struct seems to fail??
template <typename T, typename IndexSet>
struct SparseNumberArrayOutputFunctor {
  SparseNumberArrayOutputFunctor(std::ostream& o, const T* d) : _out(o), _data(d) {}

  template <typename ValueType>
  inline void operator()() const {
    _out << ", (" << ValueType::value << ',' <<
            _data[IndexSet::template IndexOf<ValueType>::index] << ')';
  }

private:
  std::ostream& _out;
  const T* _data;
};


template <typename T, typename IndexSet>
inline
std::ostream&      
operator<< (std::ostream& output, const SparseNumberArray<T, IndexSet>& a)
{
  // Enclose the entire output in braces
  output << '{';

  // Output the first value from a non-empty set
  // All values are given as ordered (index, value) pairs
  if (IndexSet::size)
    output << '(' << IndexSet::head_type::value << ',' <<
              a.raw_at(0) << ')';

  // Output the comma-separated subsequent values from a non-singleton
  // set
  if (IndexSet::size > 1)
    typename IndexSet::tail_set::ForEach()
      (SparseNumberArrayOutputFunctor<T,typename IndexSet::tail_set>(output, a.raw_data()+1));

  output << '}';
  return output;
}


// CompareTypes, RawType, ValueType specializations

#define SparseNumberArray_comparisons(templatename, settype) \
template<typename T, typename IndexSet, bool reverseorder> \
struct templatename<SparseNumberArray<T,IndexSet>, SparseNumberArray<T,IndexSet>, reverseorder> { \
  typedef SparseNumberArray<T,IndexSet> supertype; \
}; \
 \
template<typename T, typename T2, typename IndexSet, typename IndexSet2, bool reverseorder> \
struct templatename<SparseNumberArray<T,IndexSet>, SparseNumberArray<T2,IndexSet2>, reverseorder> { \
  typedef SparseNumberArray<typename Symmetric##templatename<T, T2, reverseorder>::supertype, \
                            typename IndexSet::template settype<IndexSet2>::type> supertype; \
}; \
 \
template<typename T, typename T2, typename IndexSet, bool reverseorder> \
struct templatename<SparseNumberArray<T, IndexSet>, T2, reverseorder, \
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> { \
  typedef SparseNumberArray<typename Symmetric##templatename<T, T2, reverseorder>::supertype, IndexSet> supertype; \
}

SparseNumberArray_comparisons(CompareTypes, Union);
SparseNumberArray_comparisons(PlusType, Union);
SparseNumberArray_comparisons(MinusType, Union);
SparseNumberArray_comparisons(MultipliesType, Intersection);
SparseNumberArray_comparisons(DividesType, First);
SparseNumberArray_comparisons(AndType, Intersection);
SparseNumberArray_comparisons(OrType, Union);


template <typename T, typename IndexSet>
struct RawType<SparseNumberArray<T, IndexSet> >
{
  typedef SparseNumberArray<typename RawType<T>::value_type, IndexSet> value_type;

  static value_type value(const SparseNumberArray<T, IndexSet>& a)
    {
      value_type returnval;
      for (unsigned int i=0; i != IndexSet::size; ++i)
        returnval.raw_at(i) = RawType<T>::value(a.raw_at(i));
      return returnval;
    }
};

template <typename T, typename IndexSet>
struct ValueType<SparseNumberArray<T, IndexSet> >
{
  typedef typename ValueType<T>::type type;
};

} // namespace MetaPhysicL


namespace std {

using MetaPhysicL::SparseNumberArray;
using MetaPhysicL::SymmetricCompareTypes;
using MetaPhysicL::UnaryVectorFunctor;
using MetaPhysicL::BinaryVectorFunctor;
using MetaPhysicL::call_traits;
using MetaPhysicL::binary_bind2nd;

#define SparseNumberArray_std_unary(funcname) \
template <typename T, typename IndexSet> \
inline \
SparseNumberArray<T, IndexSet> \
funcname (SparseNumberArray<T, IndexSet> a) \
{ \
  for (unsigned int i=0; i != IndexSet::size; ++i) \
    a.raw_at(i) = std::funcname(a.raw_at(i)); \
 \
  return a; \
}


#define SparseNumberArray_std_binary(funcname) \
template <typename T, typename T2, typename IndexSet, typename IndexSet2> \
inline \
SparseNumberArray<typename SymmetricCompareTypes<T,T2>::supertype, IndexSet> \
funcname (const SparseNumberArray<T, IndexSet>& a, const SparseNumberArray<T2, IndexSet2>& b) \
{ \
  typedef typename SymmetricCompareTypes<T,T2>::supertype TS; \
  SparseNumberArray<TS, IndexSet> returnval; \
 \
  typename IndexSet::ForEach() \
    (BinaryVectorFunctor<std::pointer_to_binary_function<TS,TS,TS>, \
     IndexSet,IndexSet2,IndexSet,T,T2,TS> \
      (MetaPhysicL::binary_ptr_fun(std::funcname<TS>), \
       a.raw_data(), b.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
} \
 \
template <typename T, typename T2, typename IndexSet> \
inline \
SparseNumberArray<typename SymmetricCompareTypes<T,T2>::supertype, IndexSet> \
funcname (const SparseNumberArray<T, IndexSet>& a, const T2& b) \
{ \
  typedef typename SymmetricCompareTypes<T,T2>::supertype TS; \
  SparseNumberArray<TS, IndexSet> returnval; \
 \
  typename IndexSet::ForEach() \
    (UnaryVectorFunctor<std::unary_function<TS,TS>,IndexSet,IndexSet,T,TS> \
      (std::bind2nd(MetaPhysicL::binary_ptr_fun(std::funcname<TS>),b), \
       a.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
} \
 \
template <typename T, typename T2, typename IndexSet> \
inline \
SparseNumberArray<typename SymmetricCompareTypes<T,T2>::supertype, IndexSet> \
funcname (const T& a, const SparseNumberArray<T2, IndexSet>& b) \
{ \
  typedef typename SymmetricCompareTypes<T,T2>::supertype TS; \
  SparseNumberArray<TS, IndexSet> returnval; \
 \
  typename IndexSet::ForEach() \
    (UnaryVectorFunctor<std::unary_function<TS,TS>,IndexSet,IndexSet,T2,TS> \
      (std::bind1st(MetaPhysicL::binary_ptr_fun(std::funcname<TS>),a), \
       b.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
}


#define SparseNumberArray_std_binary_union(funcname) \
template <typename T, typename T2, typename IndexSet, typename IndexSet2> \
inline \
SparseNumberArray<typename SymmetricCompareTypes<T,T2>::supertype, \
                  typename IndexSet::template Union<IndexSet2>::type> \
funcname (const SparseNumberArray<T, IndexSet>& a, const SparseNumberArray<T2, IndexSet2>& b) \
{ \
  typedef typename SymmetricCompareTypes<T,T2>::supertype TS; \
  typedef typename IndexSet::template Union<IndexSet2>::type IndexSetS; \
  SparseNumberArray<TS, IndexSetS> returnval; \
 \
  const TS& (*unambiguous) (const TS&, const TS&); \
  unambiguous = std::funcname<TS>; \
 \
  typename IndexSet::template Intersection<IndexSet2>::type::ForEach() \
    (BinaryVectorFunctor<std::pointer_to_binary_function<const TS&,const TS&,const TS&>,IndexSet,IndexSet2,IndexSetS,T,T2,TS> \
      (MetaPhysicL::binary_ptr_fun(unambiguous), \
       a.raw_data(), b.raw_data(), returnval.raw_data())); \
  typename IndexSet::template Difference<IndexSet2>::type::ForEach() \
    (UnaryVectorFunctor<MetaPhysicL::bound_second<std::pointer_to_binary_function<const TS&,const TS&,const TS&> >,IndexSet,IndexSetS,T,TS> \
      (MetaPhysicL::binary_bind2nd(MetaPhysicL::binary_ptr_fun(unambiguous),TS(0)), \
       a.raw_data(), returnval.raw_data())); \
  typename IndexSet2::template Difference<IndexSet>::type::ForEach() \
    (UnaryVectorFunctor<MetaPhysicL::bound_first<std::pointer_to_binary_function<const TS&,const TS&,const TS&> >,IndexSet2,IndexSetS,T2,TS> \
      (MetaPhysicL::binary_bind1st(MetaPhysicL::binary_ptr_fun(unambiguous),TS(0)), \
       b.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
} \
 \
template <typename T, typename IndexSet> \
inline \
SparseNumberArray<T, IndexSet> \
funcname (const SparseNumberArray<T, IndexSet>& a, const SparseNumberArray<T, IndexSet>& b) \
{ \
  SparseNumberArray<T, IndexSet> returnval; \
 \
  typename IndexSet::ForEach() \
    (BinaryVectorFunctor<std::pointer_to_binary_function<const T&,const T&,const T&>,IndexSet,IndexSet,IndexSet,T,T,T> \
      (MetaPhysicL::binary_ptr_fun(std::funcname<T>), \
       a.raw_data(), b.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
} \
 \
template <typename T, typename T2, typename IndexSet> \
inline \
SparseNumberArray<typename SymmetricCompareTypes<T,T2>::supertype, IndexSet> \
funcname (const SparseNumberArray<T, IndexSet>& a, const T2& b) \
{ \
  typedef typename SymmetricCompareTypes<T,T2>::supertype TS; \
  SparseNumberArray<TS, IndexSet> returnval; \
 \
  const TS& (*unambiguous) (const TS&, const TS&); \
  unambiguous = std::funcname<TS>; \
 \
  typename IndexSet::ForEach() \
    (UnaryVectorFunctor<MetaPhysicL::bound_second<std::pointer_to_binary_function<const TS&,const TS&,const TS&> >,IndexSet,IndexSet,T,TS> \
      (MetaPhysicL::binary_bind2nd(MetaPhysicL::binary_ptr_fun(unambiguous),TS(b)), \
       a.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
} \
 \
template <typename T, typename T2, typename IndexSet> \
inline \
SparseNumberArray<typename SymmetricCompareTypes<T,T2>::supertype, IndexSet> \
funcname (const T& a, const SparseNumberArray<T2, IndexSet>& b) \
{ \
  typedef typename SymmetricCompareTypes<T,T2>::supertype TS; \
  SparseNumberArray<TS, IndexSet> returnval; \
 \
  const TS& (*unambiguous) (const TS&, const TS&); \
  unambiguous = std::funcname<TS>; \
 \
  typename IndexSet::ForEach() \
    (UnaryVectorFunctor<MetaPhysicL::bound_first<std::pointer_to_binary_function<const TS&,const TS&,const TS&> >,IndexSet,IndexSet,T2,TS> \
      (MetaPhysicL::binary_bind1st(MetaPhysicL::binary_ptr_fun(unambiguous),TS(a)), \
       b.raw_data(), returnval.raw_data())); \
 \
  return returnval; \
}


// We can't use decltype without requiring C++11, we can't infer
// function types without decltype, and we can't declare a
// pointer_to_binary_function without a function type.  So let's make
// our own intermediate function.  This should allow us to use
// std::pow(foo,int) without conversions.
template <typename TP, typename TP2>
inline
typename SymmetricCompareTypes<TP,TP2>::supertype
sna_pow(TP tp, TP2 tp2)
{
  return std::pow(tp, tp2);
}

// Pow needs its own specialization, both to avoid being confused by
// pow<T1,T2> and because pow(x,0) isn't 0.
template <typename T, typename T2, typename IndexSet>
inline
SparseNumberArray<typename SymmetricCompareTypes<T,T2>::supertype, IndexSet>
pow (const SparseNumberArray<T, IndexSet>& a, const T2& b)
{
  typedef typename call_traits<T>::pow_param_type TP;
  typedef typename call_traits<T2>::pow_param_type TP2;
  typedef typename SymmetricCompareTypes<TP,TP2>::supertype TS;
  SparseNumberArray<TS, IndexSet> returnval;

  typedef std::pointer_to_binary_function<TP, TP2, TS> binary_functype;
  typedef MetaPhysicL::bound_second<binary_functype> unary_functype;

  typename IndexSet::ForEach()
    (UnaryVectorFunctor<unary_functype,IndexSet,IndexSet,T,TS>
      (binary_bind2nd(MetaPhysicL::binary_ptr_fun
         (static_cast<TS (*)(TP, TP2)>(sna_pow)),b),
       a.raw_data(), returnval.raw_data()));

  return returnval;
}


// NOTE: unary functions for which f(0) != 0 are undefined compile-time
// errors, because there's no efficient way to have them make sense in
// the sparse context.

// SparseNumberArray_std_binary(pow) // separate definition
// SparseNumberArray_std_unary(exp)
// SparseNumberArray_std_unary(log)
// SparseNumberArray_std_unary(log10)
SparseNumberArray_std_unary(sin)
// SparseNumberArray_std_unary(cos)
SparseNumberArray_std_unary(tan)
SparseNumberArray_std_unary(asin)
// SparseNumberArray_std_unary(acos)
SparseNumberArray_std_unary(atan)
SparseNumberArray_std_binary_union(atan2)
SparseNumberArray_std_unary(sinh)
// SparseNumberArray_std_unary(cosh)
SparseNumberArray_std_unary(tanh)
SparseNumberArray_std_unary(sqrt)
SparseNumberArray_std_unary(abs)
SparseNumberArray_std_unary(fabs)
SparseNumberArray_std_binary_union(max)
SparseNumberArray_std_binary_union(min)
SparseNumberArray_std_unary(ceil)
SparseNumberArray_std_unary(floor)
SparseNumberArray_std_binary(fmod) // dangerous unless y is dense


template <typename T, typename IndexSet>
class numeric_limits<SparseNumberArray<T, IndexSet> > : 
  public MetaPhysicL::raw_numeric_limits<SparseNumberArray<T, IndexSet>, T> {};

} // namespace std


#endif // METAPHYSICL_SPARSENUMBERARRAY_H
