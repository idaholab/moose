//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATHUTILS_H
#define MATHUTILS_H

#include "Moose.h"
#include "libmesh/libmesh.h"
#include "libmesh/utility.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/compare_types.h"

namespace MathUtils
{

inline Real
round(Real x)
{
  return ::round(x); // use round from math.h
}

inline Real
sign(Real x)
{
  return x >= 0.0 ? 1.0 : -1.0;
}

Real poly1Log(Real x, Real tol, int deriv);
Real poly2Log(Real x, Real tol, int deriv);
Real poly3Log(Real x, Real tol, int order);
Real poly4Log(Real x, Real tol, int order);
Real taylorLog(Real x);

Real pow(Real x, int e);

inline Real
heavyside(Real x)
{
  return x < 0.0 ? 0.0 : 1.0;
}
inline Real
positivePart(Real x)
{
  return x > 0.0 ? x : 0.0;
}
inline Real
negativePart(Real x)
{
  return x < 0.0 ? x : 0.0;
}

template <typename T,
          typename T2,
          typename T3,
          typename std::enable_if<ScalarTraits<T>::value && ScalarTraits<T2>::value &&
                                      ScalarTraits<T3>::value,
                                  int>::type = 0>
void
addScaled(const T & a, const T2 & b, T3 & result)
{
  result += a * b;
}

template <typename T,
          typename T2,
          typename T3,
          typename std::enable_if<ScalarTraits<T>::value, int>::type = 0>
void
addScaled(const T & scalar, const NumericVector<T2> & numeric_vector, NumericVector<T3> & result)
{
  result.add(scalar, numeric_vector);
}

template <
    typename T,
    typename T2,
    template <typename> class W,
    template <typename> class W2,
    typename std::enable_if<std::is_same<typename W<T>::index_type, unsigned int>::value &&
                                std::is_same<typename W2<T2>::index_type, unsigned int>::value,
                            int>::type = 0>
typename CompareTypes<T, T2>::supertype
dotProduct(const W<T> & a, const W2<T2> & b)
{
  return a * b;
}

template <typename T,
          typename T2,
          template <typename> class W,
          template <typename> class W2,
          typename std::enable_if<std::is_same<typename W<T>::index_type,
                                               std::tuple<unsigned int, unsigned int>>::value &&
                                      std::is_same<typename W2<T2>::index_type,
                                                   std::tuple<unsigned int, unsigned int>>::value,
                                  int>::type = 0>
typename CompareTypes<T, T2>::supertype
dotProduct(const W<T> & a, const W2<T2> & b)
{
  return a.contract(b);
}

} // namespace MathUtils

#endif // MATHUTILS_H
