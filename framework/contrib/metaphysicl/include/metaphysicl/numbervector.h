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
// $Id$
//
//--------------------------------------------------------------------------


#ifndef METAPHYSICL_NUMBERVECTOR_H
#define METAPHYSICL_NUMBERVECTOR_H

#include <algorithm>
#include <limits>
#include <ostream>

#include "metaphysicl/compare_types.h"
#include "metaphysicl/ct_types.h"
#include "metaphysicl/metaphysicl_asserts.h"
#include "metaphysicl/raw_type.h"

namespace MetaPhysicL {

template <std::size_t N, typename T>
class NumberVector;

template<std::size_t N, typename S, typename T, bool reverseorder>
struct DotType<NumberVector<N,S>, NumberVector<N,T>, reverseorder> {
  typedef typename MultipliesType<S,T,reverseorder>::supertype supertype;
};

template<std::size_t size1, std::size_t size2, typename S, typename T, bool reverseorder>
struct OuterProductType<NumberVector<size1,S>, NumberVector<size2,T>, reverseorder> {
  typedef 
  NumberVector<size1, NumberVector<size2,
    typename MultipliesType<S,T,reverseorder>::supertype> > supertype;
};

template<std::size_t N, typename S>
struct SumType<NumberVector<N,S> > {
  typedef S supertype;
};

template <std::size_t N, typename T>
class NumberVector
{
public:
  typedef T value_type;

  template <std::size_t i>
  struct entry_type {
    typedef value_type type;
  };

  template <typename T2>
  struct rebind {
    typedef NumberVector<N, T2> other;
  };

  NumberVector() {}

  NumberVector(const T& val)
    { std::fill(_data, _data+N, val); }

  NumberVector(const T* vals)
    { std::copy(vals, vals+N, _data); }

  template <typename T2>
  NumberVector(NumberVector<N, T2> src)
    { if (N) std::copy(&src[0], &src[0]+N, _data); }

  template <typename T2>
  NumberVector(const T2& val)
    { std::fill(_data, _data+N, T(val)); }

  T& operator[](std::size_t i)
    { return _data[i]; }

  const T& operator[](std::size_t i) const
    { return _data[i]; }

  T& raw_at (std::size_t i)
    { return _data[i]; }

  const T& raw_at (std::size_t i) const
    { return _data[i]; }

  template <std::size_t i>
  typename entry_type<i>::type& get()
    { return _data[i]; }

  template <std::size_t i>
  const typename entry_type<i>::type& get() const
    { return _data[i]; }

  template <std::size_t i>
  typename entry_type<i>::type& insert()
    { return _data[i]; }

  std::size_t size() const
    { return N; }

  NumberVector<N,T> operator- () const {
    NumberVector<N,T> returnval;
    for (std::size_t i=0; i != N; ++i) returnval[i] = -_data[i];
    return returnval;
  }

  NumberVector<N,T> operator! () const {
    NumberVector<N,T> returnval;
    for (std::size_t i=0; i != N; ++i) returnval[i] = !_data[i];
    return returnval;
  }

  template <typename T2>
  NumberVector<N,T>& operator+= (const NumberVector<N,T2>& a)
    { for (std::size_t i=0; i != N; ++i) _data[i] += a[i]; return *this; }

  template <typename T2>
  NumberVector<N,T>& operator+= (const T2& a)
    { for (std::size_t i=0; i != N; ++i) _data[i] += a; return *this; }

  template <typename T2>
  NumberVector<N,T>& operator-= (const NumberVector<N,T2>& a)
    { for (std::size_t i=0; i != N; ++i) _data[i] -= a[i]; return *this; }

  template <typename T2>
  NumberVector<N,T>& operator-= (const T2& a)
    { for (std::size_t i=0; i != N; ++i) _data[i] -= a; return *this; }

  template <typename T2>
  NumberVector<N,T>& operator*= (const NumberVector<N,T2>& a)
    { for (std::size_t i=0; i != N; ++i) _data[i] *= a[i]; return *this; }

  template <typename T2>
  NumberVector<N,T>& operator*= (const T2& a)
    { for (std::size_t i=0; i != N; ++i) _data[i] *= a; return *this; }

  template <typename T2>
  NumberVector<N,T>& operator/= (const NumberVector<N,T2>& a)
    { for (std::size_t i=0; i != N; ++i) _data[i] /= a[i]; return *this; }

  template <typename T2>
  NumberVector<N,T>& operator/= (const T2& a)
    { for (std::size_t i=0; i != N; ++i) _data[i] /= a; return *this; }

  template <typename T2>
  typename SymmetricMultipliesType<T,T2>::supertype
  dot (const NumberVector<N,T2>& a) const
  {
    typename SymmetricMultipliesType<T,T2>::supertype returnval = 0;
    for (std::size_t i=0; i != N; ++i)
      returnval += _data[i] * a[i];
    return returnval;
  }

  template <typename T2>
  NumberVector<N, NumberVector<N, typename SymmetricMultipliesType<T,T2>::supertype> >
  outerproduct (const NumberVector<N,T2>& a) const
  {
    NumberVector<N, NumberVector<N, typename SymmetricMultipliesType<T,T2>::supertype> > returnval;

    for (std::size_t i=0; i != N; ++i)
      for (std::size_t j=0; j != N; ++j)
        returnval[i][j] = _data[i] * a[j];

    return returnval;
  }

  static NumberVector<N, NumberVector<N, T> >
  identity(std::size_t n = N)
  {
    metaphysicl_assert_equal_to(n, N);

    NumberVector<N, NumberVector<N, T> > returnval(0);
  
    for (std::size_t i=0; i != N; ++i)
      returnval[i][i] = 1;

    return returnval;
  }


private:
  T _data[N];
};



//
// Non-member functions
//

template <std::size_t N, typename B, typename T, typename T2>
inline
typename CompareTypes<T,T2>::supertype
if_else (const NumberVector<N,B> & condition, const T & if_true, const T2 & if_false)
{
  typedef typename CompareTypes<T,T2>::supertype returntype;
  for (std::size_t i = 0; i != N; ++i)
    if (condition[i])
      return returntype(if_true);

  return returntype(if_false);
}



template <std::size_t N,
          unsigned int index1=0, typename Data1=void,
          unsigned int index2=0, typename Data2=void,
          unsigned int index3=0, typename Data3=void,
          unsigned int index4=0, typename Data4=void,
          unsigned int index5=0, typename Data5=void,
          unsigned int index6=0, typename Data6=void,
          unsigned int index7=0, typename Data7=void,
          unsigned int index8=0, typename Data8=void>
struct NumberVectorOf
{
  typedef
  typename CompareTypes<Data1,
    typename CompareTypes<Data2,
      typename CompareTypes<Data3,
        typename CompareTypes<Data4,
          typename CompareTypes<Data5,
            typename CompareTypes<Data6,
              typename CompareTypes<Data7,Data8>::supertype
            >::supertype
          >::supertype
        >::supertype
      >::supertype
    >::supertype
  >::supertype supertype;

  typedef NumberVector<N, supertype> type;
};



template <std::size_t N, std::size_t index, typename T>
struct NumberVectorUnitVector
{
  typedef NumberVector<N, T> type;

  static type value() {
    type returnval = 0;
    returnval[index] = 1;
    return returnval;
  }
};


template <std::size_t N, typename T>
struct NumberVectorFullVector
{
  typedef NumberVector<N,T> type;

  static type value() {
    type returnval;
    for (std::size_t i=0; i != N; ++i)
      returnval[i] = 1;
    return returnval;
  }
};



template <std::size_t N, typename T>
inline
NumberVector<N, NumberVector<N, T> >
transpose(NumberVector<N, NumberVector<N, T> > a)
{
  for (std::size_t i=0; i != N; ++i)
    for (std::size_t j=i+1; j != N; ++j)
      std::swap(a[i][j], a[j][i]);

  return a;
}


template<std::size_t N, typename T>
T sum (const NumberVector<N, T> &a)
{
  T returnval = 0;
  
  for (std::size_t i=0; i != N; ++i)
    returnval += a[i];

  return returnval;
}



#define NumberVector_op_ab(opname, atype, btype, newtype) \
template <std::size_t N, typename T, typename T2> \
inline \
typename newtype::supertype \
operator opname (const atype& a, const btype& b) \
{ \
  typedef typename newtype::supertype TS; \
  TS returnval(a); \
  returnval opname##= b; \
  return returnval; \
}

#define NumberVector_op(opname, typecomparison) \
NumberVector_op_ab(opname, NumberVector<N MacroComma T>, NumberVector<N MacroComma T2>, \
                  typecomparison##Type<NumberVector<N MacroComma T> MacroComma NumberVector<N MacroComma T2> >) \
NumberVector_op_ab(opname,                             T , NumberVector<N MacroComma T2>, \
                  typecomparison##Type<NumberVector<N MacroComma T2> MacroComma T MacroComma true>) \
NumberVector_op_ab(opname, NumberVector<N MacroComma T>,                             T2 , \
                  typecomparison##Type<NumberVector<N MacroComma T> MacroComma T2>)

NumberVector_op(+,Plus)
NumberVector_op(-,Minus)
NumberVector_op(*,Multiplies)
NumberVector_op(/,Divides)


#define NumberVector_operator_binary_abab(opname, atype, btype, aarg, barg) \
template <std::size_t N, typename T, typename T2> \
inline \
NumberVector<N, bool> \
operator opname (const atype& a, const btype& b) \
{ \
  NumberVector<N, bool> returnval; \
 \
  for (std::size_t i=0; i != N; ++i) \
    returnval[i] = (aarg opname barg); \
 \
  return returnval; \
}

#define NumberVector_operator_binary(opname) \
NumberVector_operator_binary_abab(opname, NumberVector<N MacroComma T>, NumberVector<N MacroComma T2>, a[i], b[i]) \
NumberVector_operator_binary_abab(opname,                             T , NumberVector<N MacroComma T2>, a,    b[i]) \
NumberVector_operator_binary_abab(opname, NumberVector<N MacroComma T>,                             T2 , a[i], b)

NumberVector_operator_binary(<)
NumberVector_operator_binary(<=)
NumberVector_operator_binary(>)
NumberVector_operator_binary(>=)
NumberVector_operator_binary(==)
NumberVector_operator_binary(!=)
NumberVector_operator_binary(&&)
NumberVector_operator_binary(||)

template <std::size_t N, typename T>
inline
std::ostream&      
operator<< (std::ostream& output, const NumberVector<N,T>& a)
{
  output << '{';
  if (N)
    output << a[0];
  for (std::size_t i=1; i<N; ++i)
    output << ',' << a[i];
  output << '}';
  return output;
}


// CompareTypes, RawType, ValueType specializations

#define NumberVector_comparisons(templatename) \
template<std::size_t N, typename T, bool reverseorder> \
struct templatename<NumberVector<N,T>, NumberVector<N,T>, reverseorder> { \
  typedef NumberVector<N, T> supertype; \
}; \
 \
template<std::size_t N, typename T, bool reverseorder> \
struct templatename<NumberVector<N,T>, NullType, reverseorder> { \
  typedef NumberVector<N, T> supertype; \
}; \
 \
template<std::size_t N, typename T, bool reverseorder> \
struct templatename<NullType, NumberVector<N,T>, reverseorder> { \
  typedef NumberVector<N, T> supertype; \
}; \
 \
template<std::size_t N, typename T, typename T2, bool reverseorder> \
struct templatename<NumberVector<N,T>, NumberVector<N,T2>, reverseorder> { \
  typedef NumberVector<N, typename Symmetric##templatename<T, T2, reverseorder>::supertype> supertype; \
}; \
 \
template<std::size_t N, std::size_t N2, typename T, typename T2, bool reverseorder> \
struct templatename<NumberVector<N,T>, NumberVector<N2,T2>, reverseorder> { \
  typedef NumberVector<0, int> supertype; \
}; \
 \
template<std::size_t N, typename T, typename T2, bool reverseorder> \
struct templatename<NumberVector<N, T>, T2, reverseorder, \
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> { \
  typedef NumberVector<N, typename Symmetric##templatename<T, T2, reverseorder>::supertype> supertype; \
}

NumberVector_comparisons(CompareTypes);
NumberVector_comparisons(PlusType);
NumberVector_comparisons(MinusType);
NumberVector_comparisons(MultipliesType);
NumberVector_comparisons(DividesType);
NumberVector_comparisons(AndType);
NumberVector_comparisons(OrType);

template <std::size_t N, typename T>
struct RawType<NumberVector<N, T> >
{
  typedef NumberVector<N, typename RawType<T>::value_type> value_type;

  static value_type value(const NumberVector<N, T>& a)
    {
      value_type returnval;
      for (std::size_t i=0; i != N; ++i)
        returnval[i] = RawType<T>::value(a[i]);
      return returnval;
    }
};

template <std::size_t N, typename T>
struct ValueType<NumberVector<N, T> >
{
  typedef typename ValueType<T>::type type;
};

} // namespace MetaPhysicL


namespace std {

using MetaPhysicL::NumberVector;
using MetaPhysicL::CompareTypes;

#define NumberVector_std_unary(funcname) \
template <std::size_t N, typename T> \
inline \
NumberVector<N, T> \
funcname (NumberVector<N, T> a) \
{ \
  for (std::size_t i=0; i != N; ++i) \
    a[i] = std::funcname(a[i]); \
 \
  return a; \
}


#define NumberVector_std_binary_abab(funcname, atype, btype, abtypes, aarg, barg) \
template <std::size_t N, typename T, typename T2> \
inline \
typename CompareTypes<abtypes>::supertype \
funcname (const atype& a, const btype& b) \
{ \
  typedef typename CompareTypes<abtypes>::supertype TS; \
  TS returnval; \
 \
  for (std::size_t i=0; i != N; ++i) \
    returnval[i] = std::funcname(aarg, barg); \
 \
  return returnval; \
}

#define NumberVector_std_binary_aa(funcname, atype) \
template <std::size_t N, typename T> \
inline \
atype \
funcname (const atype& a, const atype& b) \
{ \
  atype returnval; \
 \
  for (std::size_t i=0; i != N; ++i) \
    returnval[i] = std::funcname(a[i], b[i]); \
 \
  return returnval; \
}


#define NumberVector_std_binary(funcname) \
NumberVector_std_binary_abab(funcname, NumberVector<N MacroComma T>, NumberVector<N MacroComma T2>, \
                            NumberVector<N MacroComma T> MacroComma NumberVector<N MacroComma T2>, a[i], b[i]) \
NumberVector_std_binary_abab(funcname,                             T , NumberVector<N MacroComma T2>, \
                            NumberVector<N MacroComma T2> MacroComma T,                              a,    b[i]) \
NumberVector_std_binary_abab(funcname, NumberVector<N MacroComma T>,                             T2 , \
                            NumberVector<N MacroComma T> MacroComma T2,                              a[i],    b) \
NumberVector_std_binary_aa(funcname, NumberVector<N MacroComma T>)

// These functions are hard to consistently define on vectors, as
// opposed to arrays, so let's not define them for now.

/*
NumberVector_std_binary(pow)
NumberVector_std_unary(exp)
NumberVector_std_unary(log)
NumberVector_std_unary(log10)
NumberVector_std_unary(sin)
NumberVector_std_unary(cos)
NumberVector_std_unary(tan)
NumberVector_std_unary(asin)
NumberVector_std_unary(acos)
NumberVector_std_unary(atan)
NumberVector_std_binary(atan2)
NumberVector_std_unary(sinh)
NumberVector_std_unary(cosh)
NumberVector_std_unary(tanh)
NumberVector_std_unary(sqrt)
NumberVector_std_unary(abs)
NumberVector_std_unary(fabs)
NumberVector_std_binary(max)
NumberVector_std_binary(min)
NumberVector_std_unary(ceil)
NumberVector_std_unary(floor)
NumberVector_std_binary(fmod)
*/


template <std::size_t N, typename T>
class numeric_limits<NumberVector<N, T> > : 
  public MetaPhysicL::raw_numeric_limits<NumberVector<N, T>, T> {};

} // namespace std


#endif // METAPHYSICL_NUMBERVECTOR_H
