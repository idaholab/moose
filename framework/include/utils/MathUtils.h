//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "libmesh/libmesh.h"
#include "libmesh/utility.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/compare_types.h"
#include "libmesh/point.h"

namespace MathUtils
{

/// std::sqrt is not constexpr, so we add sqrt(2) as a constant (used in Mandel notation)
static constexpr Real sqrt2 = 1.4142135623730951;

Real poly1Log(Real x, Real tol, unsigned int derivative_order);
Real poly2Log(Real x, Real tol, unsigned int derivative_order);
Real poly3Log(Real x, Real tol, unsigned int derivative_order);
Real poly4Log(Real x, Real tol, unsigned int derivative_order);
Real taylorLog(Real x);
/**
 * Evaluate Cartesian coordinates of any center point of a triangle given Barycentric
 * coordinates of center point and Cartesian coordinates of triangle's vertices
 * @param p0,p1,p2 are the three non-collinear vertices in Cartesian coordinates
 * @param b0,b1,b2 is the center point in barycentric coordinates with b0+b1+b2=1, e.g.
 * (1/3,1/3,1/3) for a centroid
 * @return the center point of triangle in Cartesian coordinates
 */
Point barycentricToCartesian2D(const Point & p0,
                               const Point & p1,
                               const Point & p2,
                               const Real b0,
                               const Real b1,
                               const Real b2);
/**
 * Evaluate Cartesian coordinates of any center point of a tetrahedron given Barycentric
 * coordinates of center point and Cartesian coordinates of tetrahedon's vertices
 * @param p0,p1,p2,p3 are the three non-coplanar vertices in Cartesian coordinates
 * @param b0,b1,b2,b3 is the center point in barycentric coordinates with b0+b1+b2+b3=1, e.g.
 * (1/4,1/4,1/4,1/4) for a centroid.
 * @return the center point of tetrahedron in Cartesian coordinates
 */
Point barycentricToCartesian3D(const Point & p0,
                               const Point & p1,
                               const Point & p2,
                               const Point & p3,
                               const Real b0,
                               const Real b1,
                               const Real b2,
                               const Real b3);
/**
 * Evaluate circumcenter of a triangle given three arbitrary points
 * @param p0,p1,p2 are the three non-collinear vertices in Cartesian coordinates
 * @return the circumcenter in Cartesian coordinates
 */
Point circumcenter2D(const Point & p0, const Point & p1, const Point & p2);
/**
 * Evaluate circumcenter of a tetrahedrom given four arbitrary points
 * @param p0,p1,p2,p3 are the four non-coplanar vertices in Cartesian coordinates
 * @return the circumcenter in Cartesian coordinates
 */
Point circumcenter3D(const Point & p0, const Point & p1, const Point & p2, const Point & p3);

template <typename T>
T
round(T x)
{
  return ::round(x); // use round from math.h
}

template <typename T>
T
sign(T x)
{
  return x >= 0.0 ? 1.0 : -1.0;
}

template <typename T>
T
pow(T x, int e)
{
  bool neg = false;
  T result = 1.0;

  if (e < 0)
  {
    neg = true;
    e = -e;
  }

  while (e)
  {
    // if bit 0 is set multiply the current power of two factor of the exponent
    if (e & 1)
      result *= x;

    // x is incrementally set to consecutive powers of powers of two
    x *= x;

    // bit shift the exponent down
    e >>= 1;
  }

  return neg ? 1.0 / result : result;
}

template <typename T>
T
heavyside(T x)
{
  return x < 0.0 ? 0.0 : 1.0;
}

template <typename T>
T
regularizedHeavyside(T x, Real smoothing_length)
{
  if (x <= -smoothing_length)
    return 0.0;
  else if (x < smoothing_length)
    return 0.5 * (1 + std::sin(libMesh::pi * x / 2 / smoothing_length));
  else
    return 1.0;
}

template <typename T>
T
regularizedHeavysideDerivative(T x, Real smoothing_length)
{
  if (x < smoothing_length && x > -smoothing_length)
    return 0.25 * libMesh::pi / smoothing_length *
           (std::cos(libMesh::pi * x / 2 / smoothing_length));
  else
    return 0.0;
}

template <typename T>
T
positivePart(T x)
{
  return x > 0.0 ? x : 0.0;
}

template <typename T>
T
negativePart(T x)
{
  return x < 0.0 ? x : 0.0;
}

template <
    typename T,
    typename T2,
    typename T3,
    typename std::enable_if<libMesh::ScalarTraits<T>::value && libMesh::ScalarTraits<T2>::value &&
                                libMesh::ScalarTraits<T3>::value,
                            int>::type = 0>
void
addScaled(const T & a, const T2 & b, T3 & result)
{
  result += a * b;
}

template <typename T,
          typename T2,
          typename T3,
          typename std::enable_if<libMesh::ScalarTraits<T>::value, int>::type = 0>
void
addScaled(const T & scalar,
          const libMesh::NumericVector<T2> & numeric_vector,
          libMesh::NumericVector<T3> & result)
{
  result.add(scalar, numeric_vector);
}

template <
    typename T,
    typename T2,
    template <typename>
    class W,
    template <typename>
    class W2,
    typename std::enable_if<std::is_same<typename W<T>::index_type, unsigned int>::value &&
                                std::is_same<typename W2<T2>::index_type, unsigned int>::value,
                            int>::type = 0>
typename libMesh::CompareTypes<T, T2>::supertype
dotProduct(const W<T> & a, const W2<T2> & b)
{
  return a * b;
}

template <typename T,
          typename T2,
          template <typename>
          class W,
          template <typename>
          class W2,
          typename std::enable_if<std::is_same<typename W<T>::index_type,
                                               std::tuple<unsigned int, unsigned int>>::value &&
                                      std::is_same<typename W2<T2>::index_type,
                                                   std::tuple<unsigned int, unsigned int>>::value,
                                  int>::type = 0>
typename libMesh::CompareTypes<T, T2>::supertype
dotProduct(const W<T> & a, const W2<T2> & b)
{
  return a.contract(b);
}

/**
 * Evaluate a polynomial with the coefficients c at x. Note that the Polynomial
 * form is
 *   c[0]*x^s + c[1]*x^(s-1) + c[2]*x^(s-2) + ... + c[s-2]*x^2 + c[s-1]*x + c[s]
 * where s = c.size()-1 , which is counter intuitive!
 *
 * This function will be DEPRECATED soon (10/22/2020)
 *
 * The coefficient container type can be any container that provides an index
 * operator [] and a .size() method (e.g. std::vector, std::array). The return
 * type is the supertype of the container value type and the argument x.
 * The supertype is the type that can represent both number types.
 */
template <typename C,
          typename T,
          typename R = typename libMesh::CompareTypes<typename C::value_type, T>::supertype>
R
poly(const C & c, const T x, const bool derivative = false)
{
  const auto size = c.size();
  if (size == 0)
    return 0.0;

  R value = c[0];
  if (derivative)
  {
    value *= size - 1;
    for (std::size_t i = 1; i < size - 1; ++i)
      value = value * x + c[i] * (size - i - 1);
  }
  else
  {
    for (std::size_t i = 1; i < size; ++i)
      value = value * x + c[i];
  }

  return value;
}

/**
 * Evaluate a polynomial with the coefficients c at x. Note that the Polynomial
 * form is
 *   c[0] + c[1] * x + c[2] * x^2 + ...
 * The coefficient container type can be any container that provides an index
 * operator [] and a .size() method (e.g. std::vector, std::array). The return
 * type is the supertype of the container value type and the argument x.
 * The supertype is the type that can represent both number types.
 */
template <typename C,
          typename T,
          typename R = typename libMesh::CompareTypes<typename C::value_type, T>::supertype>
R
polynomial(const C & c, const T x)
{
  auto size = c.size();
  if (size == 0)
    return 0.0;

  size--;
  R value = c[size];
  for (std::size_t i = 1; i <= size; ++i)
    value = value * x + c[size - i];

  return value;
}

/**
 * Returns the derivative of polynomial(c, x) with respect to x
 */
template <typename C,
          typename T,
          typename R = typename libMesh::CompareTypes<typename C::value_type, T>::supertype>
R
polynomialDerivative(const C & c, const T x)
{
  auto size = c.size();
  if (size <= 1)
    return 0.0;

  size--;
  R value = c[size] * size;
  for (std::size_t i = 1; i < size; ++i)
    value = value * x + c[size - i] * (size - i);

  return value;
}

template <typename T, typename T2>
T
clamp(const T & x, T2 lowerlimit, T2 upperlimit)
{
  if (x < lowerlimit)
    return lowerlimit;
  if (x > upperlimit)
    return upperlimit;
  return x;
}

template <typename T, typename T2>
T
smootherStep(T x, T2 start, T2 end, bool derivative = false)
{
  mooseAssert("start < end", "Start value must be lower than end value for smootherStep");
  if (x <= start)
    return 0.0;
  else if (x >= end)
  {
    if (derivative)
      return 0.0;
    else
      return 1.0;
  }
  x = (x - start) / (end - start);
  if (derivative)
    return 30.0 * libMesh::Utility::pow<2>(x) * (x * (x - 2.0) + 1.0) / (end - start);
  return libMesh::Utility::pow<3>(x) * (x * (x * 6.0 - 15.0) + 10.0);
}

enum class ComputeType
{
  value,
  derivative
};

template <ComputeType compute_type, typename X, typename S, typename E>
auto
smootherStep(const X & x, const S & start, const E & end)
{
  mooseAssert("start < end", "Start value must be lower than end value for smootherStep");
  if (x <= start)
    return 0.0;
  else if (x >= end)
  {
    if constexpr (compute_type == ComputeType::derivative)
      return 0.0;
    if constexpr (compute_type == ComputeType::value)
      return 1.0;
  }
  const auto u = (x - start) / (end - start);
  if constexpr (compute_type == ComputeType::derivative)
    return 30.0 * libMesh::Utility::pow<2>(u) * (u * (u - 2.0) + 1.0) / (end - start);
  if constexpr (compute_type == ComputeType::value)
    return libMesh::Utility::pow<3>(u) * (u * (u * 6.0 - 15.0) + 10.0);
}

/**
 * Helper function templates to set a variable to zero.
 * Specializations may have to be implemented (for examples see
 * RankTwoTensor, RankFourTensor).
 */
template <typename T>
inline void
mooseSetToZero(T & v)
{
  /**
   * The default for non-pointer types is to assign zero.
   * This should either do something sensible, or throw a compiler error.
   * Otherwise the T type is designed badly.
   */
  v = 0;
}
template <typename T>
inline void
mooseSetToZero(T *&)
{
  mooseError("mooseSetToZero does not accept pointers");
}

template <>
inline void
mooseSetToZero(std::vector<Real> & vec)
{
  for (auto & v : vec)
    v = 0.;
}

/**
 * generate a complete multi index table for given dimension and order
 * i.e. given dim = 2, order = 2, generated table will have the following content
 * 0 0
 * 1 0
 * 0 1
 * 2 0
 * 1 1
 * 0 2
 * The first number in each entry represents the order of the first variable, i.e. x;
 * The second number in each entry represents the order of the second variable, i.e. y.
 * Multiplication is implied between numbers in each entry, i.e. 1 1 represents x^1 * y^1
 *
 * @param dim dimension of the multi-index, here dim = mesh dimension
 * @param order generate the multi-index up to certain order
 * @return a data structure holding entries representing the complete multi index
 */
std::vector<std::vector<unsigned int>> multiIndex(unsigned int dim, unsigned int order);

template <ComputeType compute_type, typename X, typename X1, typename X2, typename Y1, typename Y2>
auto
linearInterpolation(const X & x, const X1 & x1, const X2 & x2, const Y1 & y1, const Y2 & y2)
{
  const auto m = (y2 - y1) / (x2 - x1);
  if constexpr (compute_type == ComputeType::derivative)
    return m;
  if constexpr (compute_type == ComputeType::value)
    return m * (x - x1) + y1;
}

/**
 * perform modulo operator for Euclidean division that ensures a non-negative result
 * @param dividend dividend of the modulo operation
 * @param divisor divisor of the modulo operation
 * @return the non-negative remainder when the dividend is divided by the divisor
 */
template <typename T1, typename T2>
std::size_t
euclideanMod(T1 dividend, T2 divisor)
{
  return (dividend % divisor + divisor) % divisor;
}

/**
 * automatic prefixing for naming material properties based on gradients of coupled
 * variables/functors
 */
template <typename T>
T
gradName(const T & base_prop_name)
{
  return "grad_" + base_prop_name;
}

/**
 * automatic prefixing for naming material properties based on time derivatives of coupled
 * variables/functors
 */
template <typename T>
T
timeDerivName(const T & base_prop_name)
{
  return "d" + base_prop_name + "_dt";
}

/**
 * Computes the Kronecker product of two matrices.
 * @param product Reference to the product matrix
 * @param mat_A Reference to the first matrix
 * @param mat_B Reference to the other matrix
 */
void kron(RealEigenMatrix & product, const RealEigenMatrix & mat_A, const RealEigenMatrix & mat_B);

} // namespace MathUtils

/// A helper function for MathUtils::multiIndex
std::vector<std::vector<unsigned int>> multiIndexHelper(unsigned int N, unsigned int K);
