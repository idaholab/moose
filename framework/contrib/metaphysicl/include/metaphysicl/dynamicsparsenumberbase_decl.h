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


#ifndef METAPHYSICL_DYNAMICSPARSENUMBERBASE_DECL_H
#define METAPHYSICL_DYNAMICSPARSENUMBERBASE_DECL_H

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <ostream>
#include <vector>

#include "metaphysicl/compare_types.h"
#include "metaphysicl/ct_set.h"
#include "metaphysicl/metaphysicl_asserts.h"
#include "metaphysicl/raw_type.h"
#include "metaphysicl/sparsenumberutils.h"
#include "metaphysicl/testable.h"

namespace MetaPhysicL {

// Data type T, index type I
template <typename T, typename I, template <typename, typename> class SubType>
class DynamicSparseNumberBase
{
public:
  typedef T value_type;

  template <unsigned int i>
  struct entry_type {
    typedef value_type type;
  };

  typedef I index_value_type;

  std::size_t size() const;

  void resize(std::size_t s);

  DynamicSparseNumberBase();

#if __cplusplus >= 201103L
  // Move constructors are useful when all your data is on the heap
  DynamicSparseNumberBase(DynamicSparseNumberBase<T, I, SubType> && src) = default;

  // Move assignment avoids heap operations too
  DynamicSparseNumberBase& operator= (DynamicSparseNumberBase<T, I, SubType> && src) = default;

  // Standard copy operations get implicitly deleted upon move
  // constructor definition, so we manually enable them.
  DynamicSparseNumberBase(const DynamicSparseNumberBase<T, I, SubType> & src) = default;

  DynamicSparseNumberBase& operator= (const DynamicSparseNumberBase<T, I, SubType> & src) = default;
#endif

  template <typename T2, typename I2>
  DynamicSparseNumberBase(const DynamicSparseNumberBase<T2, I2, SubType> & src);

  T* raw_data();

  const T* raw_data() const;

  typename std::vector<T>::reference raw_at(unsigned int i);

  typename std::vector<T>::const_reference raw_at(unsigned int i) const;

  I& raw_index(unsigned int i);

  const I& raw_index(unsigned int i) const;

  // FIXME: these encapsulation violations are necessary for std::pow
  // until I can figure out the right friend declaration.
  const std::vector<T>& nude_data() const;

  std::vector<T>& nude_data();

  const std::vector<I>& nude_indices() const;

  std::vector<I>& nude_indices();

  std::size_t runtime_index_query(index_value_type i) const;

  std::size_t runtime_index_of(index_value_type i) const;

  T& operator[](index_value_type i);

  const T& operator[](index_value_type i) const;

  T query(index_value_type i) const;

  template <unsigned int i>
  typename entry_type<i>::type& get();

  template <unsigned int i>
  const typename entry_type<i>::type& get() const;

  value_type& insert(unsigned int i);

  template <unsigned int i>
  typename entry_type<i>::type& insert();

  template <unsigned int i, typename T2>
  void set(const T2& val);

  bool boolean_test() const;

  SubType<T,I> operator- () const;

  // Since this is a dynamically allocated sparsity pattern, we can
  // increase it as needed to support e.g. operator+=
  template <typename I2>
  void sparsity_union (const std::vector<I2>& new_indices);

  // Since this is a dynamically allocated sparsity pattern, we can
  // decrease it when possible for efficiency
  template <typename I2>
  void sparsity_intersection (const std::vector<I2>& new_indices);

  // Since this is a dynamically allocated sparsity pattern, we can
  // decrease it when possible for efficiency
  void sparsity_trim ();

  // Not defineable since !0 != 0
  // SubType<T,I> operator! () const;

  template <typename T2, typename I2>
  SubType<T,I>&
    operator+= (const SubType<T2,I2>& a);

  template <typename T2, typename I2>
  SubType<T,I>&
    operator-= (const SubType<T2,I2>& a);

  template <typename T2, typename I2>
  SubType<T,I>&
    operator*= (const SubType<T2,I2>& a);

  template <typename T2, typename I2>
  SubType<T,I>&
    operator/= (const SubType<T2,I2>& a);

  template <typename T2>
  SubType<T,I>& operator*= (const T2& a);

  template <typename T2>
  SubType<T,I>& operator/= (const T2& a);

protected:

  std::vector<T> _data;
  std::vector<I> _indices;
};


//
// Non-member functions
//

template <template <typename, typename> class SubType,
          typename B, typename IB,
          typename T, typename I,
          typename T2, typename I2>
inline
SubType<typename CompareTypes<T,T2>::supertype,
        typename CompareTypes<IB,I2>::supertype>
if_else (const DynamicSparseNumberBase<B, IB,SubType> & condition,
         const DynamicSparseNumberBase<T, I, SubType> & if_true,
         const DynamicSparseNumberBase<T2,I2,SubType> & if_false);



#define DynamicSparseNumberBase_decl_op_ab(opname, atype, btype, functorname) \
template <typename T, typename T2, typename I, typename I2> \
inline \
typename Symmetric##functorname##Type<atype,btype>::supertype \
operator opname (const atype& a, const btype& b);


#if __cplusplus >= 201103L

#define DynamicSparseNumberBase_decl_op(subtypename, opname, functorname) \
DynamicSparseNumberBase_decl_op_ab(opname, subtypename<T MacroComma I>, subtypename<T2 MacroComma I2>, functorname) \
 \
template <typename T, typename T2, typename I, typename I2> \
inline \
typename Symmetric##functorname##Type<subtypename<T,I>,subtypename<T2,I2> >::supertype \
operator opname (subtypename<T,I>&& a, \
                 const subtypename<T2,I2>& b);

#else

#define DynamicSparseNumberBase_decl_op(subtypename, opname, functorname) \
DynamicSparseNumberBase_decl_op_ab(opname, subtypename<T MacroComma I>, subtypename<T2 MacroComma I2>, functorname)

#endif

// Let's also allow scalar times vector.
// Scalar plus vector, etc. remain undefined in the sparse context.

template <template <typename, typename> class SubType,
          typename T, typename T2, typename I>
inline
typename MultipliesType<SubType<T2,I>,T,true>::supertype
operator * (const T& a, const DynamicSparseNumberBase<T2,I,SubType>& b);

template <template <typename, typename> class SubType,
          typename T, typename T2, typename I>
inline
typename MultipliesType<SubType<T,I>,T2>::supertype
operator * (const DynamicSparseNumberBase<T,I,SubType>& a, const T2& b);

template <template <typename, typename> class SubType,
          typename T, typename T2, typename I>
inline
typename DividesType<SubType<T,I>,T2>::supertype
operator / (const DynamicSparseNumberBase<T,I,SubType>& a, const T2& b);

#if __cplusplus >= 201103L
template <template <typename, typename> class SubType,
          typename T, typename T2, typename I>
inline
typename MultipliesType<SubType<T,I>,T2>::supertype
operator * (DynamicSparseNumberBase<T,I,SubType>&& a, const T2& b);

template <template <typename, typename> class SubType,
          typename T, typename T2, typename I>
inline
typename DividesType<SubType<T,I>,T2>::supertype
operator / (DynamicSparseNumberBase<T,I,SubType>&& a, const T2& b);
#endif


#define DynamicSparseNumberBase_decl_operator_binary(opname, functorname) \
template <template <typename, typename> class SubType, \
          typename T, typename T2, typename I, typename I2> \
inline \
SubType<bool, typename CompareTypes<I,I2>::supertype> \
operator opname (const DynamicSparseNumberBase<T,I,SubType>& a, \
                 const DynamicSparseNumberBase<T2,I2,SubType>& b); \
 \
template <template <typename, typename> class SubType, \
          typename T, typename T2, typename I> \
inline \
SubType<bool, I> \
operator opname (const DynamicSparseNumberBase<T,I,SubType>& a, const T2& b); \
 \
template <template <typename, typename> class SubType, \
          typename T, typename T2, typename I> \
inline \
SubType<bool, I> \
operator opname (const T& a, const DynamicSparseNumberBase<T2,I,SubType>& b);

// NOTE: unary functions for which 0-op-0 is true are undefined compile-time
// errors, because there's no efficient way to have them make sense in
// the sparse context.

DynamicSparseNumberBase_decl_operator_binary(<, less)
// DynamicSparseNumberBase_decl_operator_binary(<=)
DynamicSparseNumberBase_decl_operator_binary(>, greater)
// DynamicSparseNumberBase_decl_operator_binary(>=)
// DynamicSparseNumberBase_decl_operator_binary(==)
DynamicSparseNumberBase_decl_operator_binary(!=, not_equal_to)

// FIXME - make && an intersection rather than a union for efficiency
DynamicSparseNumberBase_decl_operator_binary(&&, logical_and)
DynamicSparseNumberBase_decl_operator_binary(||, logical_or)


template <template <typename, typename> class SubType,
          typename T, typename I>
inline
std::ostream&
operator<< (std::ostream& output, const DynamicSparseNumberBase<T,I,SubType>& a);


// CompareTypes, RawType, ValueType specializations

#define DynamicSparseNumberBase_comparisons(subtypename, templatename) \
template<typename T, typename I, bool reverseorder> \
struct templatename<subtypename<T,I>, subtypename<T,I>, reverseorder> { \
  typedef subtypename<T,I> supertype; \
}; \
 \
template<typename T, typename T2, typename I, typename I2, bool reverseorder> \
struct templatename<subtypename<T,I>, subtypename<T2,I2>, reverseorder> { \
  typedef subtypename<typename Symmetric##templatename<T, T2, reverseorder>::supertype, \
                      typename CompareTypes<I,I2>::supertype> supertype; \
}; \
 \
template<typename T, typename T2, typename I, bool reverseorder> \
struct templatename<subtypename<T, I>, T2, reverseorder, \
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> { \
  typedef subtypename<typename Symmetric##templatename<T, T2, reverseorder>::supertype, I> supertype; \
}

} // namespace MetaPhysicL

namespace std {

using MetaPhysicL::CompareTypes;
using MetaPhysicL::DynamicSparseNumberBase;
using MetaPhysicL::SymmetricCompareTypes;

#define DynamicSparseNumberBase_decl_std_unary(funcname) \
template <template <typename, typename> class SubType, \
          typename T, typename I> \
inline \
SubType<T, I> \
funcname (const DynamicSparseNumberBase<T,I,SubType> & a);


#define DynamicSparseNumberBase_decl_std_binary_union(funcname) \
template <template <typename, typename> class SubType, \
          typename T, typename T2, typename I, typename I2> \
inline \
SubType<typename SymmetricCompareTypes<T,T2>::supertype, \
        typename CompareTypes<I,I2>::supertype> \
funcname (const DynamicSparseNumberBase<T,I,SubType>& a, \
          const DynamicSparseNumberBase<T2,I2,SubType>& b); \
 \
 \
template <template <typename, typename> class SubType, \
          typename T, typename T2, typename I> \
inline \
SubType<typename SymmetricCompareTypes<T,T2>::supertype, I> \
funcname (const DynamicSparseNumberBase<T,I,SubType>& a, const T2& b); \
 \
 \
template <template <typename, typename> class SubType, \
          typename T, typename T2, typename I> \
inline \
SubType<typename SymmetricCompareTypes<T,T2>::supertype, I> \
funcname (const T& a, const DynamicSparseNumberBase<T2,I,SubType>& b);


// Pow needs its own specialization, both to avoid being confused by
// pow<T1,T2> and because pow(x,0) isn't 0.
template <template <typename, typename> class SubType, \
          typename T, typename T2, typename I>
inline
SubType<typename SymmetricCompareTypes<T,T2>::supertype, I>
pow (const DynamicSparseNumberBase<T,I,SubType>& a, const T2& b);


// NOTE: unary functions for which f(0) != 0 are undefined compile-time
// errors, because there's no efficient way to have them make sense in
// the sparse context.

// DynamicSparseNumberBase_decl_std_binary(pow) // separate definition
// DynamicSparseNumberBase_decl_std_unary(exp)
// DynamicSparseNumberBase_decl_std_unary(log)
// DynamicSparseNumberBase_decl_std_unary(log10)
DynamicSparseNumberBase_decl_std_unary(sin)
// DynamicSparseNumberBase_decl_std_unary(cos)
DynamicSparseNumberBase_decl_std_unary(tan)
DynamicSparseNumberBase_decl_std_unary(asin)
// DynamicSparseNumberBase_decl_std_unary(acos)
DynamicSparseNumberBase_decl_std_unary(atan)
DynamicSparseNumberBase_decl_std_binary_union(atan2)
DynamicSparseNumberBase_decl_std_unary(sinh)
// DynamicSparseNumberBase_decl_std_unary(cosh)
DynamicSparseNumberBase_decl_std_unary(tanh)
DynamicSparseNumberBase_decl_std_unary(sqrt)
DynamicSparseNumberBase_decl_std_unary(abs)
DynamicSparseNumberBase_decl_std_unary(fabs)
DynamicSparseNumberBase_decl_std_binary_union(max)
DynamicSparseNumberBase_decl_std_binary_union(min)
DynamicSparseNumberBase_decl_std_unary(ceil)
DynamicSparseNumberBase_decl_std_unary(floor)
DynamicSparseNumberBase_decl_std_binary_union(fmod) // TODO: optimize this

} // namespace std


#endif // METAPHYSICL_DYNAMICSPARSENUMBERARRAY_DECL_H
