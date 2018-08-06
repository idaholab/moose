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


#ifndef METAPHYSICL_DUALEXPRESSION_H
#define METAPHYSICL_DUALEXPRESSION_H

#include <ostream>
#include <limits>

#include "metaphysicl/compare_types.h"
#include "metaphysicl/ct_types.h"
#include "metaphysicl/dualderivatives.h"
#include "metaphysicl/metaprogramming.h"
#include "metaphysicl/raw_type.h"
#include "metaphysicl/testable.h"

namespace MetaPhysicL {

template <typename T, typename D=T>
class DualExpression : public safe_bool<DualExpression<T,D> >
{
private:
  T _val;
  D _deriv;

public:
  typedef T value_type;

  typedef D derivatives_type;

  DualExpression();

  template <typename T2>
  DualExpression(const T2& val);

  template <typename T2, typename D2>
  DualExpression(const T2& val, const D2& deriv);

  template <typename T2, typename D2>
  DualExpression(const T2& val, D2& deriv);

  T& value() { return _val; }

  const T& value() const { return _val; }

  D& derivatives() { return _deriv; }

  const D& derivatives() const { return _deriv; }

  bool boolean_test() const { return _val; }

  auto
  operator- () 
  -> DualExpression<decltype(-this->_val), decltype(-this->_deriv)>
  const {
    return DualExpression<decltype(-_val),decltype(-_deriv)>
      (-_val, -_deriv);
  }

  auto
  operator! () 
  -> DualExpression<decltype(!this->_val), decltype(!this->_deriv)>
  const {
    return DualExpression<decltype(!_val),decltype(!_deriv)>
      (!_val, !_deriv);
  }

  template <typename T2, typename D2>
  DualExpression<T,D>& operator+= (const DualExpression<T2,D2>& a);

  template <typename T2>
  DualExpression<T,D>& operator+= (const T2& a);

  template <typename T2, typename D2>
  DualExpression<T,D>& operator-= (const DualExpression<T2,D2>& a);

  template <typename T2>
  DualExpression<T,D>& operator-= (const T2& a);

  template <typename T2, typename D2>
  DualExpression<T,D>& operator*= (const DualExpression<T2,D2>& a);

  template <typename T2>
  DualExpression<T,D>& operator*= (const T2& a);

  template <typename T2, typename D2>
  DualExpression<T,D>& operator/= (const DualExpression<T2,D2>& a);

  template <typename T2>
  DualExpression<T,D>& operator/= (const T2& a);
};



// Helper class to handle partial specialization for DualExpression
// constructors

template <typename T, typename D>
struct DualExpressionConstructor
{
  static T value(const DualExpression<T,D>& v) { return v.value(); }

  template <typename T2>
  static T value(const T2& v) { return v; }

  template <typename T2, typename D2>
  static T value(const T2& v, const D2&) { return v; }

  template <typename T2, typename D2>
  static T value(const DualExpression<T2,D2>& v) {
    return DualExpressionConstructor<T,D>::value(v.value());
  }

  template <typename T2>
  static D deriv(const T2&) { return 0.; }

  template <typename T2, typename D2>
  static D deriv(const DualExpression<T2,D2>& v) { return v.derivatives(); }

  template <typename T2, typename D2>
  static D deriv(T2&, D2& d) { return d; }
};

template <typename T, typename D, typename DD>
struct DualExpressionConstructor<DualExpression<T,D>, DD>
{
  template <typename T2, typename D2, typename D3>
  static DualExpression<T,D> value(const DualExpression<DualExpression<T2,D2>, D3>& v) { return v.value(); }

  template <typename T2>
  static DualExpression<T,D> value(const T2& v) { return v; }

  template <typename T2, typename D2>
  static DualExpression<T,D> value(const T2& v, const D2& d) { return DualExpression<T,D>(v,d); }

  template <typename D2>
  static DualExpression<T,D> value(const DualExpression<T,D>& v, const D2&) { return v; }

  template <typename T2>
  static DD deriv(const T2&) { return 0; }

  template <typename T2, typename D2>
  static DD deriv(const DualExpression<T2,D2>& v) { return v.derivatives(); }

  template <typename T2, typename D2>
  static DD deriv(const T2&, const D2& d) { return d; }
};


//
// Syntactic sugar
//

template <typename T, typename D>
inline
DualExpression<const T&, const D&>
make_dual_expression_reference(const T& t, const D& d)
{
  return DualExpression<const T&, const D&>(t,d);
}

template <typename T, typename D>
inline
DualExpression<T, D>
make_dual_expression_copy(const T& t, const D& d)
{
  return DualExpression<T, D>(t,d);
}



//
// Member function definitions
//

template <typename T, typename D>
inline
DualExpression<T,D>::DualExpression() :
  _val(), _deriv() {}

template <typename T, typename D>
template <typename T2>
inline
DualExpression<T,D>::DualExpression(const T2& val) :
  _val  (DualExpressionConstructor<T,D>::value(val)),
  _deriv(DualExpressionConstructor<T,D>::deriv(val)) {}

template <typename T, typename D>
template <typename T2, typename D2>
inline
DualExpression<T,D>::DualExpression(const T2& val,
                                    const D2& deriv) :
  _val  (DualExpressionConstructor<T,D>::value(val,deriv)),
  _deriv(DualExpressionConstructor<T,D>::deriv(val,deriv)) {}

template <typename T, typename D>
template <typename T2, typename D2>
inline
DualExpression<T,D>::DualExpression(const T2& val,
                                    D2& deriv) :
  _val  (DualExpressionConstructor<T,D>::value(val,deriv)),
  _deriv(DualExpressionConstructor<T,D>::deriv(val,deriv)) {}



// FIXME: these operators currently do automatic type promotion when
// encountering DualExpressions of differing levels of recursion and
// differentiability.  But what we really want is automatic type
// *demotion*, to avoid pretending we have accurate derivatives which
// we don't have.  If we could do that right then some potential
// subtle run-time user errors would turn into compile-time user
// errors.

#define DualExpression_op(opname, functorname, simplecalc, dualcalc, fullderiv, rightderiv, leftderiv) \
template <typename T, typename D> \
template <typename T2> \
inline \
DualExpression<T,D>& \
DualExpression<T,D>::operator opname##= (const T2& in) \
{ \
  simplecalc; \
  this->value() opname##= in; \
  return *this; \
} \
 \
template <typename T, typename D> \
template <typename T2, typename D2> \
inline \
DualExpression<T,D>& \
DualExpression<T,D>::operator opname##= (const DualExpression<T2,D2>& in) \
{ \
  dualcalc; \
  this->value() opname##= in.value(); \
  return *this; \
} \
 \
template <typename T, typename D, typename T2, typename D2> \
inline \
auto \
operator opname (const DualExpression<T,D>& a, const DualExpression<T2,D2>& b) \
-> DualExpression<decltype(a.value() opname b.value()), \
                  typename copy_or_reference<decltype(fullderiv)>::type> \
{ \
  return DualExpression<decltype(a.value() opname b.value()), \
                        typename copy_or_reference<decltype(fullderiv)>::type> \
    (a.value() opname b.value(), fullderiv); \
} \
 \
template <typename T, typename T2, typename D> \
inline \
auto \
operator opname (const T& a, const DualExpression<T2,D>& b) \
-> typename enable_if_c<DefinesSupertype<CompareTypes< \
         DualExpression<T2,D>, \
         T \
       > \
     >::value, \
     DualExpression<decltype(a opname b.value()), \
                    typename copy_or_reference<decltype(rightderiv)>::type> \
   >::type \
{ \
  return DualExpression<decltype(a opname b.value()), \
                        typename copy_or_reference<decltype(rightderiv)>::type> \
    (a opname b.value(), rightderiv); \
} \
 \
template <typename T, typename D, typename T2> \
inline \
auto \
operator opname (const DualExpression<T,D>& a, const T2& b) \
-> typename enable_if_c<DefinesSupertype<CompareTypes< \
         DualExpression<T,D>, \
         T2 \
       > \
     >::value, \
     DualExpression<decltype(a.value() opname b), \
                    typename copy_or_reference<decltype(leftderiv)>::type> \
   >::type \
{ \
  return DualExpression<decltype(a.value() opname b), \
                        typename copy_or_reference<decltype(leftderiv)>::type> \
    (a.value() opname b, leftderiv); \
}



DualExpression_op(+, Plus, , this->derivatives() += in.derivatives(),
  a.derivatives() + b.derivatives(),
  b.derivatives(), a.derivatives())

DualExpression_op(-, Minus, , this->derivatives() -= in.derivatives(),
  a.derivatives() - b.derivatives(),
  -b.derivatives(), a.derivatives())

DualExpression_op(*, Multiplies, this->derivatives() *= in,
  this->derivatives() *= in.value();
  this->derivatives() += this->value() * in.derivatives();,
  a.value()*b.derivatives() + a.derivatives()*b.value(),
  a*b.derivatives(),
  a.derivatives()*b)

DualExpression_op(/, Divides, this->derivatives() /= in,
  this->derivatives() /= in.value();
  this->derivatives() -= this->value()/(in.value()*in.value()) * in.derivatives();,
  (a.derivatives() - a.value()/b.value()*b.derivatives())/b.value(),
  -a/b.value()*b.derivatives()/b.value(),
  a.derivatives()/b)



#define DualExpression_compare(opname) \
template <typename T, typename D, typename T2, typename D2> \
inline \
auto \
operator opname  (const DualExpression<T,D>& a, const DualExpression<T2,D2>& b) \
-> decltype(a.value() opname b.value()) \
{ \
  return (a.value() opname b.value()); \
} \
 \
template <typename T, typename T2, typename D2> \
inline \
auto \
operator opname  (const T& a, const DualExpression<T2,D2>& b) \
-> decltype(a opname b.value()) \
{ \
  return (a opname b.value()); \
} \
 \
template <typename T, typename T2, typename D> \
inline \
auto \
operator opname  (const DualExpression<T,D>& a, const T2& b) \
-> decltype(a.value() opname b) \
{ \
  return (a.value() opname b); \
}

DualExpression_compare(>)
DualExpression_compare(>=)
DualExpression_compare(<)
DualExpression_compare(<=)
DualExpression_compare(==)
DualExpression_compare(!=)
DualExpression_compare(&&)
DualExpression_compare(||)

template <typename T, typename D>
inline
std::ostream&      
operator<< (std::ostream& output, const DualExpression<T,D>& a)
{
  return output << '(' << a.value() << ',' << a.derivatives() << ')';
}


// ScalarTraits, RawType, CompareTypes specializations

template <typename T, typename D>
struct ScalarTraits<DualExpression<T, D> >
{
  static const bool value = ScalarTraits<T>::value;
};

template <typename T, typename D>
struct RawType<DualExpression<T, D> >
{
  typedef typename RawType<T>::value_type value_type;

  static value_type value(const DualExpression<T, D>& a) { return raw_value(a.value()); }
};

template<typename T, typename T2, typename D, bool reverseorder>
struct PlusType<DualExpression<T, D>, T2, reverseorder,
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualExpression<typename SymmetricPlusType<T, T2, reverseorder>::supertype, D> supertype;
};

template<typename T, typename D, typename T2, typename D2, bool reverseorder>
struct PlusType<DualExpression<T, D>, DualExpression<T2, D2>, reverseorder> {
  typedef DualExpression<typename SymmetricPlusType<T, T2, reverseorder>::supertype,
                     typename SymmetricPlusType<D, D2, reverseorder>::supertype> supertype;
};

template<typename T, typename D>
struct PlusType<DualExpression<T, D>, DualExpression<T, D> > {
  typedef DualExpression<typename SymmetricPlusType<T,T>::supertype,
                     typename SymmetricPlusType<D,D>::supertype> supertype;
};


template<typename T, typename T2, typename D, bool reverseorder>
struct MinusType<DualExpression<T, D>, T2, reverseorder,
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualExpression<typename SymmetricMinusType<T, T2, reverseorder>::supertype, D> supertype;
};

template<typename T, typename D, typename T2, typename D2, bool reverseorder>
struct MinusType<DualExpression<T, D>, DualExpression<T2, D2>, reverseorder> {
  typedef DualExpression<typename SymmetricMinusType<T, T2, reverseorder>::supertype,
                     typename SymmetricMinusType<D, D2, reverseorder>::supertype> supertype;
};

template<typename T, typename D, bool reverseorder>
struct MinusType<DualExpression<T, D>, DualExpression<T, D>, reverseorder> {
  typedef DualExpression<typename SymmetricMinusType<T,T>::supertype,
                     typename SymmetricMinusType<D,D>::supertype> supertype;
};


template<typename T, typename T2, typename D, bool reverseorder>
struct MultipliesType<DualExpression<T, D>, T2, reverseorder,
                      typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualExpression<typename SymmetricMultipliesType<T, T2, reverseorder>::supertype,
                     typename SymmetricMultipliesType<D, T2, reverseorder>::supertype> supertype;
};

template<typename T, typename D, typename T2, typename D2, bool reverseorder>
struct MultipliesType<DualExpression<T, D>, DualExpression<T2, D2>, reverseorder> {
  typedef DualExpression<typename SymmetricMultipliesType<T, T2, reverseorder>::supertype,
                     typename SymmetricPlusType<
                       typename SymmetricMultipliesType<T, D2, reverseorder>::supertype,
                       typename SymmetricMultipliesType<D, T2, reverseorder>::supertype>::supertype
                     > supertype;
};

template<typename T, typename D, bool reverseorder>
struct MultipliesType<DualExpression<T, D>, DualExpression<T, D>, reverseorder> {
  typedef DualExpression<typename SymmetricMultipliesType<T, T, reverseorder>::supertype,
                     typename SymmetricMultipliesType<T, D, reverseorder>::supertype
                     > supertype;
};


template<typename T, typename T2, typename D>
struct DividesType<DualExpression<T, D>, T2, false,
                      typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualExpression<typename SymmetricDividesType<T, T2>::supertype,
                     typename SymmetricDividesType<D, T2>::supertype> supertype;
};

template<typename T, typename D, typename T2>
struct DividesType<DualExpression<T, D>, T2, true,
                   typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualExpression<typename SymmetricDividesType<T2, T>::supertype,
                     typename SymmetricDividesType<
                       typename SymmetricMultipliesType<T2, D>::supertype,
                       T
                     >::supertype
                    > supertype;
};


template<typename T, typename D, typename T2, typename D2>
struct DividesType<DualExpression<T, D>, DualExpression<T2, D2>, false> {
  typedef DualExpression<typename SymmetricDividesType<T, T2>::supertype,
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
struct DividesType<DualExpression<T, D>, DualExpression<T2, D2>, true> {
  typedef typename DividesType<DualExpression<T2, D2>, DualExpression<T, D>, false>::supertype supertype;
};

template<typename T, typename D>
struct DividesType<DualExpression<T, D>, DualExpression<T, D>, false> {
  typedef DualExpression<T,
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
struct DividesType<DualExpression<T, D>, DualExpression<T, D>, true> {
  typedef typename DividesType<DualExpression<T, D>, DualExpression<T, D>, false>::supertype supertype;
};

template<typename T, typename T2, typename D, bool reverseorder>
struct AndType<DualExpression<T, D>, T2, reverseorder,
               typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualExpression<typename SymmetricAndType<T, T2, reverseorder>::supertype, bool> supertype;
};

template<typename T, typename D, typename T2, typename D2, bool reverseorder>
struct AndType<DualExpression<T, D>, DualExpression<T2, D2>, reverseorder> {
  typedef DualExpression<typename SymmetricAndType<T, T2, reverseorder>::supertype,
                     bool> supertype;
};

template<typename T, typename D>
struct AndType<DualExpression<T, D>, DualExpression<T, D> > {
  typedef DualExpression<typename SymmetricAndType<T,T>::supertype,
                     bool> supertype;
};

template<typename T, typename T2, typename D, bool reverseorder>
struct OrType<DualExpression<T, D>, T2, reverseorder,
              typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef DualExpression<typename SymmetricOrType<T, T2, reverseorder>::supertype, bool> supertype;
};

template<typename T, typename D, typename T2, typename D2, bool reverseorder>
struct OrType<DualExpression<T, D>, DualExpression<T2, D2>, reverseorder> {
  typedef DualExpression<typename SymmetricOrType<T, T2, reverseorder>::supertype,
                     bool> supertype;
};

template<typename T, typename D>
struct OrType<DualExpression<T, D>, DualExpression<T, D> > {
  typedef DualExpression<typename SymmetricOrType<T,T>::supertype,
                     bool> supertype;
};



// DualExpression CompareTypes supertypes can't be made accurate, but
// we can still test for their existence to determine what types are
// comparable

template<typename T, typename T2, typename D, bool reverseorder>
struct CompareTypes<DualExpression<T, D>, T2, reverseorder,
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef bool supertype;
};

template<typename T, typename D, typename T2, typename D2>
struct CompareTypes<DualExpression<T, D>, DualExpression<T2, D2> > {
  typedef bool supertype;
};

template<typename T, typename D>
struct CompareTypes<DualExpression<T, D>, DualExpression<T, D> > {
  typedef bool supertype;
};



template <typename T, typename D>
struct
copy_or_reference<DualExpression<T, D> &>
{
  typedef typename
    IfElse<copy_or_reference<T&>::copy,
           DualExpression<T,D>,
           DualExpression<T,D>&>::type type;

  static const bool copy = copy_or_reference<T&>::copy;

  ctassert<copy_or_reference<T&>::copy ==
           copy_or_reference<D&>::copy> consistency_check;
};


template <typename T, typename D>
struct
copy_or_reference<const DualExpression<T, D> &>
{
  typedef typename
    IfElse<copy_or_reference<T&>::copy,
           DualExpression<T,D>,
           const DualExpression<T,D>&>::type type;

  static const bool copy = copy_or_reference<T&>::copy;

  ctassert<copy_or_reference<T&>::copy ==
           copy_or_reference<D&>::copy> consistency_check;
};



template <typename T, typename D>
inline
D gradient(const DualExpression<T, D>& a)
{
  return a.derivatives();
}


} // namespace MetaPhysicL


namespace std {

using MetaPhysicL::DualExpression;
using MetaPhysicL::CompareTypes;
using MetaPhysicL::enable_if_c;
using MetaPhysicL::DefinesSupertype;
using std::isnan;

template <typename T, typename D>
inline
auto isnan (const DualExpression<T,D> & a)
-> decltype(isnan(a.value()))
{
  return isnan(a.value());
}

// Some forward declarations necessary for recursive DualExpressions

template <typename T, typename D>
inline DualExpression<T,D> cos   (DualExpression<T,D> a);

template <typename T, typename D>
inline DualExpression<T,D> cosh  (DualExpression<T,D> a);

// Now just combined declaration/definitions

#define DualExpression_std_unary(funcname, derivcalc) \
template <typename T, typename D> \
inline \
auto funcname (DualExpression<T,D> in) \
-> DualExpression<decltype(std::funcname(in.value())), \
                  decltype((derivcalc)*in.derivatives())> \
{ \
  return DualExpression<decltype(std::funcname(in.value())), \
                        decltype((derivcalc)*in.derivatives())> \
    (std::funcname(in.value()), (derivcalc)*in.derivatives()); \
}

DualExpression_std_unary(sqrt, 1 / (2 * std::sqrt(in.value())))
DualExpression_std_unary(exp, std::exp(in.value()))
DualExpression_std_unary(log, 1 / in.value())
DualExpression_std_unary(log10, 1 / in.value() * (1/std::log(T(10))))
DualExpression_std_unary(sin, std::cos(in.value()))
DualExpression_std_unary(cos, -std::sin(in.value()))
DualExpression_std_unary(tan, 1 / std::cos(in.value()) / std::cos(in.value()))
DualExpression_std_unary(asin, 1 / std::sqrt(1 - in.value()*in.value()))
DualExpression_std_unary(acos, -1 / std::sqrt(1 - in.value()*in.value()))
DualExpression_std_unary(atan, 1 / (1 + in.value()*in.value()))
DualExpression_std_unary(sinh, std::cosh(in.value()))
DualExpression_std_unary(cosh, std::sinh(in.value()))
DualExpression_std_unary(tanh, 1 / std::cosh(in.value()) / std::cosh(in.value()))
DualExpression_std_unary(abs, (in.value() > 0) - (in.value() < 0)) // std < and > return 0 or 1
DualExpression_std_unary(fabs, (in.value() > 0) - (in.value() < 0)) // std < and > return 0 or 1
DualExpression_std_unary(ceil, 0)
DualExpression_std_unary(floor, 0)

#define DualExpression_std_binary(funcname, derivative, rightderiv, leftderiv) \
template <typename T, typename D, typename T2, typename D2> \
inline \
auto \
funcname (const DualExpression<T,D>& a, const DualExpression<T2,D2>& b) \
-> DualExpression<decltype(std::funcname(a.value(), b.value())), \
                  typename MetaPhysicL::copy_or_reference<decltype(derivative)>::type> \
{ \
  return DualExpression<decltype(std::funcname(a.value(), b.value())), \
                        typename MetaPhysicL::copy_or_reference<decltype(derivative)>::type> \
    (std::funcname(a.value(), b.value()), derivative); \
} \
 \
template <typename T, typename D> \
inline \
auto \
funcname (const DualExpression<T,D>& a, const DualExpression<T,D>& b) \
-> DualExpression<decltype(std::funcname(a.value(), b.value())), \
                  typename MetaPhysicL::copy_or_reference<decltype(derivative)>::type> \
{ \
  return DualExpression<decltype(std::funcname(a.value(), b.value())), \
                        typename MetaPhysicL::copy_or_reference<decltype(derivative)>::type> \
    (std::funcname(a.value(), b.value()), derivative); \
} \
 \
template <typename T, typename T2, typename D> \
inline \
auto \
funcname (const T& a, const DualExpression<T2,D>& b) \
-> typename enable_if_c<DefinesSupertype<CompareTypes< \
         DualExpression<T2,D>, \
         T \
       > \
     >::value, \
     DualExpression<decltype(std::funcname(a, b.value())), \
                    typename MetaPhysicL::copy_or_reference<decltype(rightderiv)>::type> \
   >::type \
{ \
  return DualExpression<decltype(std::funcname(a, b.value())), \
                        typename MetaPhysicL::copy_or_reference<decltype(rightderiv)>::type> \
    (std::funcname(a, b.value()), rightderiv); \
} \
 \
template <typename T, typename T2, typename D> \
inline \
auto \
funcname (const DualExpression<T,D>& a, const T2& b) \
-> typename enable_if_c<DefinesSupertype<CompareTypes< \
         DualExpression<T,D>, \
         T2 \
       > \
     >::value, \
     DualExpression<decltype(std::funcname(a.value(), b)), \
                    typename MetaPhysicL::copy_or_reference<decltype(leftderiv)>::type> \
   >::type \
{ \
  return DualExpression<decltype(std::funcname(a.value(), b)), \
                        typename MetaPhysicL::copy_or_reference<decltype(leftderiv)>::type> \
    (std::funcname(a.value(), b), leftderiv); \
}

// if_else is necessary here to handle cases where a is negative but
// b' is 0; we should have a contribution of 0 from those, not NaN.
DualExpression_std_binary(pow,
  std::pow(a.value(), b.value()) * (b.value() * a.derivatives() / a.value() +
  MetaPhysicL::if_else(b.derivatives(), b.derivatives() * std::log(a.value()), b.derivatives())),
  std::pow(a, b.value()) *
  MetaPhysicL::if_else(b.derivatives(), (b.derivatives() * std::log(a)), b.derivatives()),
  std::pow(a.value(), b) * (b * a.derivatives() / a.value())
  )
DualExpression_std_binary(atan2,
  (b.value() * a.derivatives() - a.value() * b.derivatives()) /
  (b.value() * b.value() + a.value() * a.value()),
  (-a * b.derivatives()) /
  (b.value() * b.value() + a * a),
  (b * a.derivatives()) /
  (b * b + a.value() * a.value()))
DualExpression_std_binary(max,
  (a.value() > b.value()) ? a.derivatives() : b.derivatives(),
  (a > b.value()) ? 0 : b.derivatives(),
  (a.value() > b) ? a.derivatives() : 0)
DualExpression_std_binary(min,
  (a.value() > b.value()) ? b.derivatives() : a.derivatives(),
  (a > b.value()) ? b.derivatives() : 0,
  (a.value() > b) ? 0 : a.derivatives())
DualExpression_std_binary(fmod, a.derivatives(), 0, a.derivatives())

template <typename T, typename D>
class numeric_limits<DualExpression<T, D> > : 
  public MetaPhysicL::raw_numeric_limits<DualExpression<T, D>, T> {};

} // namespace std


#endif // METAPHYSICL_DUALEXPRESSION_H
