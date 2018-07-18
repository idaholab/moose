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


#ifndef METAPHYSICL_DYNAMICSPARSENUMBERVECTOR_DECL_H
#define METAPHYSICL_DYNAMICSPARSENUMBERVECTOR_DECL_H

#include "metaphysicl/dynamicsparsenumberbase_decl.h"

namespace MetaPhysicL {

// Forward declarations

// Data type T, index type I
template <typename T, typename I>
class DynamicSparseNumberVector;

// Helper structs

template<typename I1, typename I2, typename S, typename T, bool reverseorder>
struct DotType<DynamicSparseNumberVector<S,I1>,
               DynamicSparseNumberVector<T,I2>, reverseorder> {
  typedef typename MultipliesType<S,T,reverseorder>::supertype supertype;
};

template<typename I1, typename I2, typename S, typename T, bool reverseorder>
struct OuterProductType<DynamicSparseNumberVector<S, I1>,
                        DynamicSparseNumberVector<T, I2>, reverseorder> {
  typedef
    DynamicSparseNumberVector<DynamicSparseNumberVector<
      typename MultipliesType<S,T,reverseorder>::supertype,
      I2>, I1> supertype;
};

template<typename S, typename I>
struct SumType<DynamicSparseNumberVector<S, I> > {
  typedef S supertype;
};


template <typename T, typename I>
class DynamicSparseNumberVector :
  public DynamicSparseNumberBase<T,I,DynamicSparseNumberVector>,
  public safe_bool<DynamicSparseNumberVector<T,I> >
{
public:
  template <typename T2>
  struct rebind {
    typedef DynamicSparseNumberVector<T2, I> other;
  };

  DynamicSparseNumberVector();

  DynamicSparseNumberVector(const T& val);

  template <typename T2>
  DynamicSparseNumberVector(const T2& val);

#if __cplusplus >= 201103L
  // Move constructors are useful when all your data is on the heap
  DynamicSparseNumberVector(DynamicSparseNumberVector<T, I> && src) = default;

  // Move assignment avoids heap operations too
  DynamicSparseNumberVector& operator= (DynamicSparseNumberVector<T, I> && src) = default;

  // Standard copy operations get implicitly deleted upon move
  // constructor definition, so we redefine them.
  DynamicSparseNumberVector(const DynamicSparseNumberVector<T, I> & src) = default;

  DynamicSparseNumberVector& operator= (const DynamicSparseNumberVector<T, I> & src) = default;
#endif

  template <typename T2, typename I2>
  DynamicSparseNumberVector(DynamicSparseNumberVector<T2, I2> src);

  template <typename T2, typename I2>
  typename MultipliesType<T,T2>::supertype
  dot (const DynamicSparseNumberVector<T2,I2>& a) const;

  template <typename T2, typename I2>
  DynamicSparseNumberVector<DynamicSparseNumberVector<
    typename MultipliesType<T,T2>::supertype,
    I2>, I>
  outerproduct (const DynamicSparseNumberVector<T2, I2>& a) const;

  static DynamicSparseNumberVector<DynamicSparseNumberVector<T, I>, I>
  identity(std::size_t n = 0);
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
struct DynamicSparseNumberVectorOf
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

  typedef DynamicSparseNumberVector<supertype, unsigned int> type;
};



template <std::size_t N, unsigned int index, typename T>
struct DynamicSparseNumberVectorUnitVector
{
  typedef DynamicSparseNumberVector<T, unsigned int> type;

  static type value() {
    type returnval;
    returnval.resize(1);
    returnval.raw_at(0) = 1;
    returnval.raw_index(0) = index;
    return returnval;
  }
};


template <std::size_t N, typename T>
struct DynamicSparseNumberVectorFullVector
{
  typedef DynamicSparseNumberVector<T,unsigned int> type;

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
DynamicSparseNumberVector<DynamicSparseNumberVector<T, I>, I2>
transpose(const DynamicSparseNumberVector<DynamicSparseNumberVector<T, I2>, I>& /*a*/);


template <typename T, typename I>
T
sum (const DynamicSparseNumberVector<T, I> &a);


DynamicSparseNumberBase_decl_op(DynamicSparseNumberVector, +, Plus)       // Union)
DynamicSparseNumberBase_decl_op(DynamicSparseNumberVector, -, Minus)      // Union)
DynamicSparseNumberBase_decl_op(DynamicSparseNumberVector, *, Multiplies) // Intersection)
DynamicSparseNumberBase_decl_op(DynamicSparseNumberVector, /, Divides)    // First)


// CompareTypes, RawType, ValueType specializations

DynamicSparseNumberBase_comparisons(DynamicSparseNumberVector, CompareTypes);
DynamicSparseNumberBase_comparisons(DynamicSparseNumberVector, PlusType);
DynamicSparseNumberBase_comparisons(DynamicSparseNumberVector, MinusType);
DynamicSparseNumberBase_comparisons(DynamicSparseNumberVector, MultipliesType);
DynamicSparseNumberBase_comparisons(DynamicSparseNumberVector, DividesType);
DynamicSparseNumberBase_comparisons(DynamicSparseNumberVector, AndType);
DynamicSparseNumberBase_comparisons(DynamicSparseNumberVector, OrType);


template <typename T, typename I>
struct RawType<DynamicSparseNumberVector<T, I> >
{
  typedef DynamicSparseNumberVector<typename RawType<T>::value_type, I> value_type;

  static value_type value(const DynamicSparseNumberVector<T, I>& a);
};

template <typename T, typename I>
struct ValueType<DynamicSparseNumberVector<T, I> >
{
  typedef typename ValueType<T>::type type;
};

} // namespace MetaPhysicL


namespace std {

using MetaPhysicL::DynamicSparseNumberVector;

template <typename T, typename I>
class numeric_limits<DynamicSparseNumberVector<T, I> > :
  public MetaPhysicL::raw_numeric_limits<DynamicSparseNumberVector<T, I>, T> {};

} // namespace std


#endif // METAPHYSICL_DYNAMICSPARSENUMBERVECTOR_DECL_H
