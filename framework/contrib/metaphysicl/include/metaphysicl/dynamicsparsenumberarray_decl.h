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


#ifndef METAPHYSICL_DYNAMICSPARSENUMBERARRAY_DECL_H
#define METAPHYSICL_DYNAMICSPARSENUMBERARRAY_DECL_H

#include "metaphysicl/dynamicsparsenumberbase_decl.h"

namespace MetaPhysicL {

// Forward declarations

// Data type T, index type I
template <typename T, typename I>
class DynamicSparseNumberArray;

// Helper structs

template<typename I1, typename I2, typename S, typename T, bool reverseorder>
struct DotType<DynamicSparseNumberArray<S,I1>,
               DynamicSparseNumberArray<T,I2>, reverseorder> {
  typedef
    DynamicSparseNumberArray
      <typename DotType<S,T,reverseorder>::supertype,
       typename CompareTypes<I1, I2>::supertype>
      supertype;
};

template<typename I1, typename I2, typename S, typename T, bool reverseorder>
struct OuterProductType<DynamicSparseNumberArray<S, I1>,
                        DynamicSparseNumberArray<T, I2>, reverseorder> {
  typedef
    DynamicSparseNumberArray
      <typename OuterProductType<S,T,reverseorder>::supertype,
       typename CompareTypes<I1, I2>::supertype>
      supertype;
};

template<typename S, typename I>
struct SumType<DynamicSparseNumberArray<S, I> > {
  typedef DynamicSparseNumberArray<typename SumType<S>::supertype, I> supertype;
};


template <typename T, typename I>
class DynamicSparseNumberArray :
  public DynamicSparseNumberBase<T,I,DynamicSparseNumberArray>,
  public safe_bool<DynamicSparseNumberArray<T,I> >
{
public:
  template <typename T2>
  struct rebind {
    typedef DynamicSparseNumberArray<T2, I> other;
  };

  DynamicSparseNumberArray();

  DynamicSparseNumberArray(const T& val);

  template <typename T2>
  DynamicSparseNumberArray(const T2& val);

#if __cplusplus >= 201103L
  // Move constructors are useful when all your data is on the heap
  DynamicSparseNumberArray(DynamicSparseNumberArray<T, I> && src) = default;

  // Move assignment avoids heap operations too
  DynamicSparseNumberArray& operator= (DynamicSparseNumberArray<T, I> && src) = default;

  // Standard copy operations get implicitly deleted upon move
  // constructor definition, so we manually enable them.
  DynamicSparseNumberArray(const DynamicSparseNumberArray<T, I> & src) = default;

  DynamicSparseNumberArray& operator= (const DynamicSparseNumberArray<T, I> & src) = default;
#endif

  template <typename T2, typename I2>
  DynamicSparseNumberArray(DynamicSparseNumberArray<T2, I2> src);

  template <typename T2, typename I2>
  DynamicSparseNumberArray
    <typename DotType<T,T2>::supertype,
     typename CompareTypes<I, I2>::supertype>
  dot (const DynamicSparseNumberArray<T2,I2>& a) const;

  template <typename T2, typename I2>
  DynamicSparseNumberArray<
    typename OuterProductType<T,T2>::supertype,
    typename CompareTypes<I, I2>::supertype>
  outerproduct (const DynamicSparseNumberArray<T2, I2>& a) const;
};


//
// Non-member functions
//

template <unsigned int N,
          unsigned int index1=0, typename Data1=void,
          unsigned int index2=0, typename Data2=void,
          unsigned int index3=0, typename Data3=void,
          unsigned int index4=0, typename Data4=void,
          unsigned int index5=0, typename Data5=void,
          unsigned int index6=0, typename Data6=void,
          unsigned int index7=0, typename Data7=void,
          unsigned int index8=0, typename Data8=void>
struct DynamicSparseNumberArrayOf
{
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

  typedef DynamicSparseNumberArray<supertype, unsigned int> type;
};



template <std::size_t N, unsigned int index, typename T>
struct DynamicSparseNumberArrayUnitVector
{
  typedef DynamicSparseNumberArray<T, unsigned int> type;

  static type value() {
    type returnval;
    returnval.resize(1);
    returnval.raw_at(0) = 1;
    returnval.raw_index(0) = index;
    return returnval;
  }
};


template <std::size_t N, typename T>
struct DynamicSparseNumberArrayFullVector
{
  typedef DynamicSparseNumberArray<T,unsigned int> type;

  static type value() {
    type returnval;
    returnval.resize(N);
    for (unsigned int i=0; i != N; ++i)
      {
        returnval.raw_at(i) = 1;
        returnval.raw_index(i) = i;
      }
    return returnval;
  }
};



template <typename T, typename I, typename I2>
inline
DynamicSparseNumberArray<DynamicSparseNumberArray<T, I>, I2>
transpose(const DynamicSparseNumberArray<DynamicSparseNumberArray<T, I2>, I>& /*a*/);


template <typename T, typename I>
DynamicSparseNumberArray<typename SumType<T>::supertype, I>
sum (const DynamicSparseNumberArray<T, I> &a);



DynamicSparseNumberBase_decl_op(DynamicSparseNumberArray, +, Plus)       // Union)
DynamicSparseNumberBase_decl_op(DynamicSparseNumberArray, -, Minus)      // Union)
DynamicSparseNumberBase_decl_op(DynamicSparseNumberArray, *, Multiplies) // Intersection)
DynamicSparseNumberBase_decl_op(DynamicSparseNumberArray, /, Divides)    // First)


// CompareTypes, RawType, ValueType specializations

DynamicSparseNumberBase_comparisons(DynamicSparseNumberArray, CompareTypes);
DynamicSparseNumberBase_comparisons(DynamicSparseNumberArray, PlusType);
DynamicSparseNumberBase_comparisons(DynamicSparseNumberArray, MinusType);
DynamicSparseNumberBase_comparisons(DynamicSparseNumberArray, MultipliesType);
DynamicSparseNumberBase_comparisons(DynamicSparseNumberArray, DividesType);
DynamicSparseNumberBase_comparisons(DynamicSparseNumberArray, AndType);
DynamicSparseNumberBase_comparisons(DynamicSparseNumberArray, OrType);


template <typename T, typename I>
struct RawType<DynamicSparseNumberArray<T, I> >
{
  typedef DynamicSparseNumberArray<typename RawType<T>::value_type, I> value_type;

  static value_type value(const DynamicSparseNumberArray<T, I>& a);
};

template <typename T, typename I>
struct ValueType<DynamicSparseNumberArray<T, I> >
{
  typedef typename ValueType<T>::type type;
};

} // namespace MetaPhysicL


namespace std {

using MetaPhysicL::DynamicSparseNumberArray;

template <typename T, typename I>
class numeric_limits<DynamicSparseNumberArray<T, I> > :
  public MetaPhysicL::raw_numeric_limits<DynamicSparseNumberArray<T, I>, T> {};

} // namespace std


#endif // METAPHYSICL_DYNAMICSPARSENUMBERARRAY_DECL_H
