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


#ifndef METAPHYSICL_DUALNUMBER_H
#define METAPHYSICL_DUALNUMBER_H

#include "metaphysicl/dualnumber_decl.h"

namespace MetaPhysicL {

template <typename T, typename D>
inline
T&
DualNumber<T,D>::value() { return _val; }

template <typename T, typename D>
inline
const T&
DualNumber<T,D>::value() const { return _val; }

template <typename T, typename D>
inline
D&
DualNumber<T,D>::derivatives() { return _deriv; }

template <typename T, typename D>
inline
const D&
DualNumber<T,D>::derivatives() const { return _deriv; }

template <typename T, typename D>
inline
bool
DualNumber<T,D>::boolean_test() const { return _val; }

template <typename T, typename D>
inline
DualNumber<T,D> 
DualNumber<T,D>::operator- () const { return DualNumber<T,D>(-_val, -_deriv); }

template <typename T, typename D>
inline
DualNumber<T,D> 
DualNumber<T,D>::operator! () const { return DualNumber<T,D>(!_val, !_deriv); }


//
// Member function definitions
//

template <typename T, typename D>
inline
DualNumber<T,D>::DualNumber() :
  _val(), _deriv() {}

template <typename T, typename D>
template <typename T2>
inline
DualNumber<T,D>::DualNumber(const T2& val) :
  _val  (DualNumberConstructor<T,D>::value(val)),
  _deriv(DualNumberConstructor<T,D>::deriv(val)) {}

template <typename T, typename D>
template <typename T2, typename D2>
inline
DualNumber<T,D>::DualNumber(const T2& val,
                            const D2& deriv) :
  _val  (DualNumberConstructor<T,D>::value(val,deriv)),
  _deriv(DualNumberConstructor<T,D>::deriv(val,deriv)) {}


// FIXME: these operators currently do automatic type promotion when
// encountering DualNumbers of differing levels of recursion and
// differentiability.  But what we really want is automatic type
// *demotion*, to avoid pretending we have accurate derivatives which
// we don't have.  If we could do that right then some potential
// subtle run-time user errors would turn into compile-time user
// errors.

#define DualNumber_preop(opname, functorname, simplecalc, dualcalc) \
template <typename T, typename D> \
template <typename T2> \
inline \
DualNumber<T,D>& \
DualNumber<T,D>::operator opname##= (const T2& in) \
{ \
  simplecalc; \
  this->value() opname##= in; \
  return *this; \
} \
 \
template <typename T, typename D> \
template <typename T2, typename D2> \
inline \
DualNumber<T,D>& \
DualNumber<T,D>::operator opname##= (const DualNumber<T2,D2>& in) \
{ \
  dualcalc; \
  this->value() opname##= in.value(); \
  return *this; \
} \
 \
template <typename T, typename D, typename T2, typename D2> \
inline \
typename functorname##Type<DualNumber<T,D>,DualNumber<T2,D2> >::supertype \
operator opname (const DualNumber<T,D>& a, const DualNumber<T2,D2>& b) \
{ \
  typedef typename \
    functorname##Type<DualNumber<T,D>,DualNumber<T2,D2> >::supertype \
    DS; \
  DS returnval = a; \
  returnval opname##= b; \
  return returnval; \
} \
 \
template <typename T, typename T2, typename D> \
inline \
typename functorname##Type<DualNumber<T2,D>,T,true>::supertype \
operator opname (const T& a, const DualNumber<T2,D>& b) \
{ \
  typedef typename \
    functorname##Type<DualNumber<T2,D>,T,true>::supertype DS; \
  DS returnval = a; \
  returnval opname##= b; \
  return returnval; \
} \
 \
template <typename T, typename D, typename T2> \
inline \
typename functorname##Type<DualNumber<T,D>,T2,false>::supertype \
operator opname (const DualNumber<T,D>& a, const T2& b) \
{ \
  typedef typename \
    functorname##Type<DualNumber<T,D>,T2,false>::supertype DS; \
  DS returnval = a; \
  returnval opname##= b; \
  return returnval; \
}



// With C++11, define "move operations" where possible.  We should be
// more complete and define the move-from-b alternatives as well, but
// those would require additional support to correctly handle
// division, subtraction, or non-commutative addition/multiplication
#if __cplusplus >= 201103L
#define DualNumber_op(opname, functorname, simplecalc, dualcalc) \
        DualNumber_preop(opname, functorname, simplecalc, dualcalc) \
 \
template <typename T, typename D, typename T2, typename D2> \
inline \
typename functorname##Type<DualNumber<T,D>,DualNumber<T2,D2> >::supertype \
operator opname (DualNumber<T,D>&& a, const DualNumber<T2,D2>& b) \
{ \
  typedef typename \
    functorname##Type<DualNumber<T,D>,DualNumber<T2,D2> >::supertype \
    DS; \
  DS returnval = std::move(a); \
  returnval opname##= b; \
  return returnval; \
} \
 \
template <typename T, typename D, typename T2> \
inline \
typename functorname##Type<DualNumber<T,D>,T2,false>::supertype \
operator opname (DualNumber<T,D>&& a, const T2& b) \
{ \
  typedef typename \
    functorname##Type<DualNumber<T,D>,T2,false>::supertype DS; \
  DS returnval = std::move(a); \
  returnval opname##= b; \
  return returnval; \
}

#else
#define DualNumber_op(opname, functorname, simplecalc, dualcalc) \
        DualNumber_preop(opname, functorname, simplecalc, dualcalc)
#endif


DualNumber_op(+, Plus, , this->derivatives() += in.derivatives())

DualNumber_op(-, Minus, , this->derivatives() -= in.derivatives())

DualNumber_op(*, Multiplies, this->derivatives() *= in,
  this->derivatives() *= in.value();
  this->derivatives() += this->value() * in.derivatives();)

DualNumber_op(/, Divides, this->derivatives() /= in,
  this->derivatives() /= in.value();
  this->derivatives() -= this->value()/(in.value()*in.value()) * in.derivatives();
)



#define DualNumber_compare(opname) \
template <typename T, typename D, typename T2, typename D2> \
inline \
bool \
operator opname  (const DualNumber<T,D>& a, const DualNumber<T2,D2>& b) \
{ \
  return (a.value() opname b.value()); \
} \
 \
template <typename T, typename T2, typename D2> \
inline \
typename boostcopy::enable_if_class< \
  typename CompareTypes<DualNumber<T2,D2>,T>::supertype, \
  bool \
>::type \
operator opname  (const T& a, const DualNumber<T2,D2>& b) \
{ \
  return (a opname b.value()); \
} \
 \
template <typename T, typename T2, typename D> \
inline \
typename boostcopy::enable_if_class< \
  typename CompareTypes<DualNumber<T,D>,T2>::supertype, \
  bool \
>::type \
operator opname  (const DualNumber<T,D>& a, const T2& b) \
{ \
  return (a.value() opname b); \
}

DualNumber_compare(>)
DualNumber_compare(>=)
DualNumber_compare(<)
DualNumber_compare(<=)
DualNumber_compare(==)
DualNumber_compare(!=)
DualNumber_compare(&&)
DualNumber_compare(||)

template <typename T, typename D>
inline
std::ostream&      
operator<< (std::ostream& output, const DualNumber<T,D>& a)
{
  return output << '(' << a.value() << ',' << a.derivatives() << ')';
}


template <typename T, typename D>
inline
D gradient(const DualNumber<T, D>& a)
{
  return a.derivatives();
}


} // namespace MetaPhysicL


namespace std {

using MetaPhysicL::DualNumber;
using MetaPhysicL::CompareTypes;

template <typename T, typename D>
inline bool isnan (const DualNumber<T,D> & a)
{
  using std::isnan;
  return isnan(a.value());
}


#if __cplusplus >= 201103L
#define DualNumber_std_unary(funcname, derivative, precalc) \
template <typename T, typename D> \
inline \
DualNumber<T,D> funcname (const DualNumber<T,D> & in) \
{ \
  DualNumber<T,D> returnval = in; \
  T funcval = std::funcname(in.value()); \
  precalc; \
  returnval.derivatives() *= derivative; \
  returnval.value() = funcval; \
  return returnval; \
} \
 \
template <typename T, typename D> \
inline \
DualNumber<T,D> funcname (DualNumber<T,D> && in) \
{ \
  T funcval = std::funcname(in.value()); \
  precalc; \
  in.derivatives() *= derivative; \
  in.value() = funcval; \
  return in; \
}



#else

#define DualNumber_std_unary(funcname, derivative, precalc) \
template <typename T, typename D> \
inline \
DualNumber<T,D> funcname (DualNumber<T,D> in) \
{ \
  T funcval = std::funcname(in.value()); \
  precalc; \
  in.derivatives() *= derivative; \
  in.value() = funcval; \
  return in; \
}

#endif

DualNumber_std_unary(sqrt, 1 / (2 * funcval),)
DualNumber_std_unary(exp, funcval,)
DualNumber_std_unary(log, 1 / in.value(),)
DualNumber_std_unary(log10, 1 / in.value() * (1/std::log(T(10.))),)
DualNumber_std_unary(sin, std::cos(in.value()),)
DualNumber_std_unary(cos, -std::sin(in.value()),)
DualNumber_std_unary(tan, sec_in * sec_in, T sec_in = 1 / std::cos(in.value()))
DualNumber_std_unary(asin, 1 / std::sqrt(1 - in.value()*in.value()),)
DualNumber_std_unary(acos, -1 / std::sqrt(1 - in.value()*in.value()),)
DualNumber_std_unary(atan, 1 / (1 + in.value()*in.value()),)
DualNumber_std_unary(sinh, std::cosh(in.value()),)
DualNumber_std_unary(cosh, std::sinh(in.value()),)
DualNumber_std_unary(tanh, sech_in * sech_in, T sech_in = 1 / std::cosh(in.value()))
DualNumber_std_unary(abs, (in.value() > 0) - (in.value() < 0),) // std < and > return 0 or 1
DualNumber_std_unary(fabs, (in.value() > 0) - (in.value() < 0),) // std < and > return 0 or 1
DualNumber_std_unary(ceil, 0,)
DualNumber_std_unary(floor, 0,)

#define DualNumber_std_binary(funcname, derivative) \
template <typename T, typename D, typename T2, typename D2> \
inline \
typename CompareTypes<DualNumber<T,D>,DualNumber<T2,D2> >::supertype \
funcname (const DualNumber<T,D>& a, const DualNumber<T2,D2>& b) \
{ \
  typedef typename CompareTypes<T,T2>::supertype TS; \
  typedef typename CompareTypes<DualNumber<T,D>,DualNumber<T2,D2> >::supertype type; \
 \
  TS funcval = std::funcname(a.value(), b.value()); \
  return type(funcval, derivative); \
} \
 \
template <typename T, typename D> \
inline \
DualNumber<T,D> \
funcname (const DualNumber<T,D>& a, const DualNumber<T,D>& b) \
{ \
  T funcval = std::funcname(a.value(), b.value()); \
  return DualNumber<T,D>(funcval, derivative); \
} \
 \
template <typename T, typename T2, typename D> \
inline \
typename CompareTypes<DualNumber<T2,D>,T,true>::supertype \
funcname (const T& a, const DualNumber<T2,D>& b) \
{ \
  typedef typename CompareTypes<DualNumber<T2,D>,T,true>::supertype type; \
  type newa(a); \
  return std::funcname(newa, b); \
} \
 \
template <typename T, typename T2, typename D> \
inline \
typename CompareTypes<DualNumber<T,D>,T2>::supertype \
funcname (const DualNumber<T,D>& a, const T2& b) \
{ \
  typedef typename CompareTypes<DualNumber<T,D>,T2>::supertype type; \
  type newb(b); \
  return std::funcname(a, newb); \
}

// if_else is necessary here to handle cases where a is negative but b
// is 0; we should have a contribution of 0 from those, not NaN.
DualNumber_std_binary(pow,
  funcval * (b.value() * a.derivatives() / a.value() +
  MetaPhysicL::if_else(b.derivatives(), b.derivatives() * std::log(a.value()), b.derivatives())))
DualNumber_std_binary(atan2,
  (b.value() * a.derivatives() - a.value() * b.derivatives()) /
  (b.value() * b.value() + a.value() * a.value()))
DualNumber_std_binary(max,
  (a.value() > b.value()) ? a.derivatives() : b.derivatives())
DualNumber_std_binary(min,
  (a.value() > b.value()) ? b.derivatives() : a.derivatives())
DualNumber_std_binary(fmod, a.derivatives())

} // namespace std


#endif // METAPHYSICL_DUALNUMBER_H
