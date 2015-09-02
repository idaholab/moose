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


#ifndef METAPHYSICL_DUALNUMBER_DECL_H
#define METAPHYSICL_DUALNUMBER_DECL_H

#include <ostream>
#include <limits>

#include "metaphysicl/compare_types.h"
#include "metaphysicl/dualderivatives.h"
#include "metaphysicl/raw_type.h"
#include "metaphysicl/testable.h"

namespace MetaPhysicL {

template <typename T, typename D=T>
class DualNumber : public safe_bool<DualNumber<T,D> >
{
public:
  typedef T value_type;

  typedef D derivatives_type;

  DualNumber();

  template <typename T2>
  DualNumber(const T2& val);

  template <typename T2, typename D2>
  DualNumber(const T2& val, const D2& deriv);

#if __cplusplus >= 201103L
  // Move constructors are useful when all your data is on the heap
  DualNumber(DualNumber<T, D> && src) = default;

  // Move assignment avoids heap operations too
  DualNumber& operator= (DualNumber<T, D> && src) = default;

  // Standard copy operations get implicitly deleted upon move
  // constructor definition, so we redefine them.
  DualNumber(const DualNumber<T, D> & src) = default;

  DualNumber& operator= (const DualNumber<T, D> & src) = default;
#endif


  T& value();

  const T& value() const;

  D& derivatives();

  const D& derivatives() const;

  bool boolean_test() const;

  DualNumber<T,D> operator- () const;

  DualNumber<T,D> operator! () const;

  template <typename T2, typename D2>
  DualNumber<T,D>& operator+= (const DualNumber<T2,D2>& a);

  template <typename T2>
  DualNumber<T,D>& operator+= (const T2& a);

  template <typename T2, typename D2>
  DualNumber<T,D>& operator-= (const DualNumber<T2,D2>& a);

  template <typename T2>
  DualNumber<T,D>& operator-= (const T2& a);

  template <typename T2, typename D2>
  DualNumber<T,D>& operator*= (const DualNumber<T2,D2>& a);

  template <typename T2>
  DualNumber<T,D>& operator*= (const T2& a);

  template <typename T2, typename D2>
  DualNumber<T,D>& operator/= (const DualNumber<T2,D2>& a);

  template <typename T2>
  DualNumber<T,D>& operator/= (const T2& a);

private:
  T _val;
  D _deriv;
};



// Helper class to handle partial specialization for DualNumber
// constructors

template <typename T, typename D>
struct DualNumberConstructor
{
  static T value(const DualNumber<T,D>& v) { return v.value(); }

  template <typename T2>
  static T value(const T2& v) { return v; }

  template <typename T2, typename D2>
  static T value(const T2& v, const D2&) { return v; }

  template <typename T2, typename D2>
  static T value(const DualNumber<T2,D2>& v) {
    return DualNumberConstructor<T,D>::value(v.value());
  }

  template <typename T2>
  static D deriv(const T2&) { return 0.; }

  template <typename T2, typename D2>
  static D deriv(const DualNumber<T2,D2>& v) { return v.derivatives(); }

  template <typename T2, typename D2>
  static D deriv(const T2&, const D2& d) { return d; }
};

template <typename T, typename D, typename DD>
struct DualNumberConstructor<DualNumber<T,D>, DD>
{
  template <typename T2, typename D2, typename D3>
  static DualNumber<T,D> value(const DualNumber<DualNumber<T2,D2>, D3>& v) { return v.value(); }

  template <typename T2>
  static DualNumber<T,D> value(const T2& v) { return v; }

  template <typename T2, typename D2>
  static DualNumber<T,D> value(const T2& v, const D2& d) { return DualNumber<T,D>(v,d); }

  template <typename D2>
  static DualNumber<T,D> value(const DualNumber<T,D>& v, const D2&) { return v; }

  template <typename T2>
  static DD deriv(const T2&) { return 0; }

  template <typename T2, typename D2>
  static DD deriv(const DualNumber<T2,D2>& v) { return v.derivatives(); }

  template <typename T2, typename D2>
  static DD deriv(const T2&, const D2& d) { return d; }
};


// FIXME: these operators currently do automatic type promotion when
// encountering DualNumbers of differing levels of recursion and
// differentiability.  But what we really want is automatic type
// *demotion*, to avoid pretending we have accurate derivatives which
// we don't have.  If we could do that right then some potential
// subtle run-time user errors would turn into compile-time user
// errors.

#define DualNumber_decl_preop(opname, functorname) \
template <typename T, typename D, typename T2, typename D2> \
inline \
typename functorname##Type<DualNumber<T,D>,DualNumber<T2,D2> >::supertype \
operator opname (const DualNumber<T,D>& a, const DualNumber<T2,D2>& b); \
 \
 \
template <typename T, typename T2, typename D> \
inline \
typename functorname##Type<DualNumber<T2,D>,T,true>::supertype \
operator opname (const T& a, const DualNumber<T2,D>& b); \
 \
 \
template <typename T, typename D, typename T2> \
inline \
typename functorname##Type<DualNumber<T,D>,T2,false>::supertype \
operator opname (const DualNumber<T,D>& a, const T2& b);



// With C++11, define "move operations" where possible.  We should be
// more complete and define the move-from-b alternatives as well, but
// those would require additional support to correctly handle
// division, subtraction, or non-commutative addition/multiplication
#if __cplusplus >= 201103L
#define DualNumber_decl_op(opname, functorname) \
        DualNumber_decl_preop(opname, functorname) \
 \
template <typename T, typename D, typename T2, typename D2> \
inline \
typename functorname##Type<DualNumber<T,D>,DualNumber<T2,D2> >::supertype \
operator opname (DualNumber<T,D>&& a, const DualNumber<T2,D2>& b); \
 \
 \
template <typename T, typename D, typename T2> \
inline \
typename functorname##Type<DualNumber<T,D>,T2,false>::supertype \
operator opname (DualNumber<T,D>&& a, const T2& b); \

#else
#define DualNumber_decl_op(opname, functorname) \
        DualNumber_decl_preop(opname, functorname)
#endif


DualNumber_decl_op(+, Plus)
DualNumber_decl_op(-, Minus)
DualNumber_decl_op(*, Multiplies)
DualNumber_decl_op(/, Divides)


#define DualNumber_decl_compare(opname) \
template <typename T, typename D, typename T2, typename D2> \
inline \
bool \
operator opname  (const DualNumber<T,D>& a, const DualNumber<T2,D2>& b); \
 \
 \
template <typename T, typename T2, typename D2> \
inline \
typename boostcopy::enable_if_class< \
  typename CompareTypes<DualNumber<T2,D2>,T>::supertype, \
  bool \
>::type \
operator opname  (const T& a, const DualNumber<T2,D2>& b); \
 \
 \
template <typename T, typename T2, typename D> \
inline \
typename boostcopy::enable_if_class< \
  typename CompareTypes<DualNumber<T,D>,T2>::supertype, \
  bool \
>::type \
operator opname  (const DualNumber<T,D>& a, const T2& b);

DualNumber_decl_compare(>)
DualNumber_decl_compare(>=)
DualNumber_decl_compare(<)
DualNumber_decl_compare(<=)
DualNumber_decl_compare(==)
DualNumber_decl_compare(!=)
DualNumber_decl_compare(&&)
DualNumber_decl_compare(||)

template <typename T, typename D>
inline
std::ostream&      
operator<< (std::ostream& output, const DualNumber<T,D>& a);


// ScalarTraits, RawType, CompareTypes specializations

template <typename T, typename D>
struct ScalarTraits<DualNumber<T, D> >
{
  static const bool value = ScalarTraits<T>::value;
};

template <typename T, typename D>
struct RawType<DualNumber<T, D> >
{
  typedef typename RawType<T>::value_type value_type;

  static value_type value(const DualNumber<T, D>& a) { return raw_value(a.value()); }
};

template<typename T, typename T2, typename D, bool reverseorder>
struct PlusType<DualNumber<T, D>, T2, reverseorder,
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualNumber<typename SymmetricPlusType<T, T2, reverseorder>::supertype, D> supertype;
};

template<typename T, typename D, typename T2, typename D2, bool reverseorder>
struct PlusType<DualNumber<T, D>, DualNumber<T2, D2>, reverseorder> {
  typedef DualNumber<typename SymmetricPlusType<T, T2, reverseorder>::supertype,
                     typename SymmetricPlusType<D, D2, reverseorder>::supertype> supertype;
};

template<typename T, typename D>
struct PlusType<DualNumber<T, D>, DualNumber<T, D> > {
  typedef DualNumber<typename SymmetricPlusType<T,T>::supertype,
                     typename SymmetricPlusType<D,D>::supertype> supertype;
};


template<typename T, typename T2, typename D, bool reverseorder>
struct MinusType<DualNumber<T, D>, T2, reverseorder,
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualNumber<typename SymmetricMinusType<T, T2, reverseorder>::supertype, D> supertype;
};

template<typename T, typename D, typename T2, typename D2, bool reverseorder>
struct MinusType<DualNumber<T, D>, DualNumber<T2, D2>, reverseorder> {
  typedef DualNumber<typename SymmetricMinusType<T, T2, reverseorder>::supertype,
                     typename SymmetricMinusType<D, D2, reverseorder>::supertype> supertype;
};

template<typename T, typename D, bool reverseorder>
struct MinusType<DualNumber<T, D>, DualNumber<T, D>, reverseorder> {
  typedef DualNumber<typename SymmetricMinusType<T,T>::supertype,
                     typename SymmetricMinusType<D,D>::supertype> supertype;
};


template<typename T, typename T2, typename D, bool reverseorder>
struct MultipliesType<DualNumber<T, D>, T2, reverseorder,
                      typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualNumber<typename SymmetricMultipliesType<T, T2, reverseorder>::supertype,
                     typename SymmetricMultipliesType<D, T2, reverseorder>::supertype> supertype;
};

template<typename T, typename D, typename T2, typename D2, bool reverseorder>
struct MultipliesType<DualNumber<T, D>, DualNumber<T2, D2>, reverseorder> {
  typedef DualNumber<typename SymmetricMultipliesType<T, T2, reverseorder>::supertype,
                     typename SymmetricPlusType<
                       typename SymmetricMultipliesType<T, D2, reverseorder>::supertype,
                       typename SymmetricMultipliesType<D, T2, reverseorder>::supertype>::supertype
                     > supertype;
};

template<typename T, typename D, bool reverseorder>
struct MultipliesType<DualNumber<T, D>, DualNumber<T, D>, reverseorder> {
  typedef DualNumber<typename SymmetricMultipliesType<T, T, reverseorder>::supertype,
                     typename SymmetricMultipliesType<T, D, reverseorder>::supertype
                     > supertype;
};


template<typename T, typename T2, typename D>
struct DividesType<DualNumber<T, D>, T2, false,
                      typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualNumber<typename SymmetricDividesType<T, T2>::supertype,
                     typename SymmetricDividesType<D, T2>::supertype> supertype;
};

template<typename T, typename D, typename T2>
struct DividesType<DualNumber<T, D>, T2, true,
                   typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualNumber<typename SymmetricDividesType<T2, T>::supertype,
                     typename SymmetricDividesType<
                       typename SymmetricMultipliesType<T2, D>::supertype,
                       T
                     >::supertype
                    > supertype;
};


template<typename T, typename D, typename T2, typename D2>
struct DividesType<DualNumber<T, D>, DualNumber<T2, D2>, false> {
  typedef DualNumber<typename SymmetricDividesType<T, T2>::supertype,
                     typename SymmetricMinusType<
                       typename SymmetricDividesType<T2, D>::supertype,
                       typename SymmetricDividesType<
                         typename SymmetricMultipliesType<T, D2>::supertype,
                         T2
                       >::supertype
                     >::supertype
                    > supertype;
};

template<typename T, typename D, typename T2, typename D2>
struct DividesType<DualNumber<T, D>, DualNumber<T2, D2>, true> {
  typedef typename DividesType<DualNumber<T2, D2>, DualNumber<T, D>, false>::supertype supertype;
};

template<typename T, typename D>
struct DividesType<DualNumber<T, D>, DualNumber<T, D>, false> {
  typedef DualNumber<T,
                     typename SymmetricMinusType<
                       typename SymmetricDividesType<T, D>::supertype,
                       typename SymmetricDividesType<
                         typename SymmetricMultipliesType<T, D>::supertype,
                         T
                       >::supertype
                     >::supertype
                    > supertype;
};

template<typename T, typename D>
struct DividesType<DualNumber<T, D>, DualNumber<T, D>, true> {
  typedef typename DividesType<DualNumber<T, D>, DualNumber<T, D>, false>::supertype supertype;
};

template<typename T, typename T2, typename D, bool reverseorder>
struct AndType<DualNumber<T, D>, T2, reverseorder,
               typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualNumber<typename SymmetricAndType<T, T2, reverseorder>::supertype, bool> supertype;
};

template<typename T, typename D, typename T2, typename D2, bool reverseorder>
struct AndType<DualNumber<T, D>, DualNumber<T2, D2>, reverseorder> {
  typedef DualNumber<typename SymmetricAndType<T, T2, reverseorder>::supertype,
                     bool> supertype;
};

template<typename T, typename D>
struct AndType<DualNumber<T, D>, DualNumber<T, D> > {
  typedef DualNumber<typename SymmetricAndType<T,T>::supertype,
                     bool> supertype;
};

template<typename T, typename T2, typename D, bool reverseorder>
struct OrType<DualNumber<T, D>, T2, reverseorder,
              typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualNumber<typename SymmetricOrType<T, T2, reverseorder>::supertype, bool> supertype;
};

template<typename T, typename D, typename T2, typename D2, bool reverseorder>
struct OrType<DualNumber<T, D>, DualNumber<T2, D2>, reverseorder> {
  typedef DualNumber<typename SymmetricOrType<T, T2, reverseorder>::supertype,
                     bool> supertype;
};

template<typename T, typename D>
struct OrType<DualNumber<T, D>, DualNumber<T, D> > {
  typedef DualNumber<typename SymmetricOrType<T,T>::supertype,
                     bool> supertype;
};




template<typename T, typename T2, typename D, bool reverseorder>
struct CompareTypes<DualNumber<T, D>, T2, reverseorder,
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualNumber<typename SymmetricCompareTypes<T, T2>::supertype,
                     typename SymmetricCompareTypes<
                       typename SymmetricCompareTypes<D, T2>::supertype,
                       T
                     >::supertype> supertype;
};

template<typename T, typename D, typename T2, typename D2>
struct CompareTypes<DualNumber<T, D>, DualNumber<T2, D2> > {
  typedef DualNumber<typename SymmetricCompareTypes<T, T2>::supertype,
                     typename SymmetricCompareTypes<
                       typename SymmetricCompareTypes<T, T2>::supertype,
                       typename SymmetricCompareTypes<D, D2>::supertype
                     >::supertype
                    > supertype;
};

template<typename T, typename D>
struct CompareTypes<DualNumber<T, D>, DualNumber<T, D> > {
  typedef DualNumber<T, typename SymmetricCompareTypes<T, D>::supertype> supertype;
};


template <typename T, typename D>
inline
D gradient(const DualNumber<T, D>& a);


} // namespace MetaPhysicL


namespace std {

using MetaPhysicL::DualNumber;
using MetaPhysicL::CompareTypes;

template <typename T, typename D>
inline bool isnan (const DualNumber<T,D> & a);

// Some forward declarations necessary for recursive DualNumbers

#if __cplusplus >= 201103L

template <typename T, typename D>
inline DualNumber<T,D> cos  (const DualNumber<T,D> & a);

template <typename T, typename D>
inline DualNumber<T,D> cos  (DualNumber<T,D> && a);

template <typename T, typename D>
inline DualNumber<T,D> cosh (const DualNumber<T,D> & a);

template <typename T, typename D>
inline DualNumber<T,D> cosh (DualNumber<T,D> && a);

#else

template <typename T, typename D>
inline DualNumber<T,D> cos  (DualNumber<T,D> a);

template <typename T, typename D>
inline DualNumber<T,D> cosh (DualNumber<T,D> a);

#endif

// Now just combined declaration/definitions

#if __cplusplus >= 201103L
#define DualNumber_decl_std_unary(funcname) \
template <typename T, typename D> \
inline \
DualNumber<T,D> funcname (const DualNumber<T,D> & in); \
 \
 \
template <typename T, typename D> \
inline \
DualNumber<T,D> funcname (DualNumber<T,D> && in);


#else

#define DualNumber_decl_std_unary(funcname) \
template <typename T, typename D> \
inline \
DualNumber<T,D> funcname (DualNumber<T,D> in);

#endif

DualNumber_decl_std_unary(sqrt)
DualNumber_decl_std_unary(exp)
DualNumber_decl_std_unary(log)
DualNumber_decl_std_unary(log10)
DualNumber_decl_std_unary(sin)
DualNumber_decl_std_unary(cos)
DualNumber_decl_std_unary(tan)
DualNumber_decl_std_unary(asin)
DualNumber_decl_std_unary(acos)
DualNumber_decl_std_unary(atan)
DualNumber_decl_std_unary(sinh)
DualNumber_decl_std_unary(cosh)
DualNumber_decl_std_unary(tanh)
DualNumber_decl_std_unary(abs)
DualNumber_decl_std_unary(fabs)
DualNumber_decl_std_unary(ceil)
DualNumber_decl_std_unary(floor)

#define DualNumber_decl_std_binary(funcname) \
template <typename T, typename D, typename T2, typename D2> \
inline \
typename CompareTypes<DualNumber<T,D>,DualNumber<T2,D2> >::supertype \
funcname (const DualNumber<T,D>& a, const DualNumber<T2,D2>& b); \
 \
 \
template <typename T, typename D> \
inline \
DualNumber<T,D> \
funcname (const DualNumber<T,D>& a, const DualNumber<T,D>& b); \
 \
 \
template <typename T, typename T2, typename D> \
inline \
typename CompareTypes<DualNumber<T2,D>,T,true>::supertype \
funcname (const T& a, const DualNumber<T2,D>& b); \
 \
 \
template <typename T, typename T2, typename D> \
inline \
typename CompareTypes<DualNumber<T,D>,T2>::supertype \
funcname (const DualNumber<T,D>& a, const T2& b);

// if_else is necessary here to handle cases where a is negative but b
// is 0; we should have a contribution of 0 from those, not NaN.
DualNumber_decl_std_binary(pow)
DualNumber_decl_std_binary(atan2)
DualNumber_decl_std_binary(max)
DualNumber_decl_std_binary(min)
DualNumber_decl_std_binary(fmod)

template <typename T, typename D>
class numeric_limits<DualNumber<T, D> > : 
  public MetaPhysicL::raw_numeric_limits<DualNumber<T, D>, T> {};

} // namespace std


#endif // METAPHYSICL_DUALNUMBER_DECL_H
