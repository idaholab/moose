//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MathUtils.h"
#include "MooseUtils.h"

namespace MathUtils
{

void
kron(RealEigenMatrix & product, const RealEigenMatrix & mat_A, const RealEigenMatrix & mat_B)
{
  product.resize(mat_A.rows() * mat_B.rows(), mat_A.cols() * mat_B.cols());
  for (unsigned int i = 0; i < mat_A.rows(); i++)
    for (unsigned int j = 0; j < mat_A.cols(); j++)
      for (unsigned int k = 0; k < mat_B.rows(); k++)
        for (unsigned int l = 0; l < mat_B.cols(); l++)
          product(((i * mat_B.rows()) + k), ((j * mat_B.cols()) + l)) = mat_A(i, j) * mat_B(k, l);
}

Real
plainLog(Real x, unsigned int derivative_order)
{
  switch (derivative_order)
  {
    case 0:
      return std::log(x);

    case 1:
      return 1.0 / x;

    case 2:
      return -1.0 / (x * x);

    case 3:
      return 2.0 / (x * x * x);
  }

  mooseError("Unsupported derivative order ", derivative_order);
}

Real
poly1Log(Real x, Real tol, unsigned int derivative_order)
{
  if (x >= tol)
    return plainLog(x, derivative_order);

  const auto c1 = [&]() { return 1.0 / tol; };
  const auto c2 = [&]() { return std::log(tol) - 1.0; };

  switch (derivative_order)
  {
    case 0:
      return c1() * x + c2();

    case 1:
      return c1();

    case 2:
      return 0.0;

    case 3:
      return 0.0;
  }

  mooseError("Unsupported derivative order ", derivative_order);
}

Real
poly2Log(Real x, Real tol, unsigned int derivative_order)
{
  if (x >= tol)
    return plainLog(x, derivative_order);

  const auto c1 = [&]() { return -0.5 / (tol * tol); };
  const auto c2 = [&]() { return 2.0 / tol; };
  const auto c3 = [&]() { return std::log(tol) - 3.0 / 2.0; };

  switch (derivative_order)
  {
    case 0:
      return c1() * x * x + c2() * x + c3();

    case 1:
      return 2.0 * c1() * x + c2();

    case 2:
      return 2.0 * c1();

    case 3:
      return 0.0;
  }

  mooseError("Unsupported derivative order ", derivative_order);
}

Real
poly3Log(Real x, Real tol, unsigned int derivative_order)
{
  if (x >= tol)
    return plainLog(x, derivative_order);

  const auto c1 = [&]() { return 1.0 / (3.0 * tol * tol * tol); };
  const auto c2 = [&]() { return -3.0 / (2.0 * tol * tol); };
  const auto c3 = [&]() { return 3.0 / tol; };
  const auto c4 = [&]() { return std::log(tol) - 11.0 / 6.0; };

  switch (derivative_order)
  {
    case 0:
      return c1() * x * x * x + c2() * x * x + c3() * x + c4();

    case 1:
      return 3.0 * c1() * x * x + 2.0 * c2() * x + c3();

    case 2:
      return 6.0 * c1() * x + 2.0 * c2();

    case 3:
      return 6.0 * c1();
  }

  mooseError("Unsupported derivative order ", derivative_order);
}

Real
poly4Log(Real x, Real tol, unsigned int derivative_order)
{
  if (x >= tol)
    return plainLog(x, derivative_order);

  switch (derivative_order)
  {
    case 0:
      return std::log(tol) + (x - tol) / tol -
             Utility::pow<2>(x - tol) / (2.0 * Utility::pow<2>(tol)) +
             Utility::pow<3>(x - tol) / (3.0 * Utility::pow<3>(tol)) -
             Utility::pow<4>(x - tol) / (4.0 * Utility::pow<4>(tol)) +
             Utility::pow<5>(x - tol) / (5.0 * Utility::pow<5>(tol)) -
             Utility::pow<6>(x - tol) / (6.0 * Utility::pow<6>(tol));

    case 1:
      return 1.0 / tol - (x - tol) / Utility::pow<2>(tol) +
             Utility::pow<2>(x - tol) / Utility::pow<3>(tol) -
             Utility::pow<3>(x - tol) / Utility::pow<4>(tol) +
             Utility::pow<4>(x - tol) / Utility::pow<5>(tol) -
             Utility::pow<5>(x - tol) / Utility::pow<6>(tol);

    case 2:
      return -1.0 / Utility::pow<2>(tol) + 2.0 * (x - tol) / Utility::pow<3>(tol) -
             3.0 * Utility::pow<2>(x - tol) / Utility::pow<4>(tol) +
             4.0 * Utility::pow<3>(x - tol) / Utility::pow<5>(tol) -
             5.0 * Utility::pow<4>(x - tol) / Utility::pow<6>(tol);

    case 3:
      return 2.0 / Utility::pow<3>(tol) - 6.0 * (x - tol) / Utility::pow<4>(tol) +
             12.0 * Utility::pow<2>(x - tol) / Utility::pow<5>(tol) -
             20.0 * Utility::pow<3>(x - tol) / Utility::pow<6>(tol);
  }

  mooseError("Unsupported derivative order ", derivative_order);
}

/// \todo This can be done without std::pow!
Real
taylorLog(Real x)
{
  Real y = (x - 1.0) / (x + 1.0);
  Real val = 1.0;
  for (unsigned int i = 0; i < 5; ++i)
  {
    Real exponent = i + 2.0;
    val += 1.0 / (2.0 * (i + 1.0) + 1.0) * std::pow(y, exponent);
  }

  return val * 2.0 * y;
}

std::vector<std::vector<unsigned int>>
multiIndex(unsigned int dim, unsigned int order)
{
  // first row all zero
  std::vector<std::vector<unsigned int>> multi_index;
  std::vector<std::vector<unsigned int>> n_choose_k;
  std::vector<unsigned int> row(dim, 0);
  multi_index.push_back(row);

  if (dim == 1)
    for (unsigned int q = 1; q <= order; q++)
    {
      row[0] = q;
      multi_index.push_back(row);
    }
  else
    for (unsigned int q = 1; q <= order; q++)
    {
      n_choose_k = multiIndexHelper(dim + q - 1, dim - 1);
      for (unsigned int r = 0; r < n_choose_k.size(); r++)
      {
        row.clear();
        for (unsigned int c = 1; c < n_choose_k[0].size(); c++)
          row.push_back(n_choose_k[r][c] - n_choose_k[r][c - 1] - 1);
        multi_index.push_back(row);
      }
    }

  return multi_index;
}

Point
barycentricToCartesian2D(const Point & p0,
                         const Point & p1,
                         const Point & p2,
                         const Real b0,
                         const Real b1,
                         const Real b2)
{
  mooseAssert(!MooseUtils::isZero(b0 + b1 + b2 - 1.0), "Barycentric coordinates must sum to one!");

  Point center;

  for (unsigned int d = 0; d < 2; ++d)
    center(d) = p0(d) * b0 + p1(d) * b1 + p2(d) * b2;
  // p0, p1, p2 are vertices of triangle
  // b0, b1, b2 are Barycentric coordinates of the triangle center

  return center;
}

Point
barycentricToCartesian3D(const Point & p0,
                         const Point & p1,
                         const Point & p2,
                         const Point & p3,
                         const Real b0,
                         const Real b1,
                         const Real b2,
                         const Real b3)
{
  mooseAssert(!MooseUtils::isZero(b0 + b1 + b2 + b3 - 1.0),
              "Barycentric coordinates must sum to one!");

  Point center;

  for (unsigned int d = 0; d < 3; ++d)
    center(d) = p0(d) * b0 + p1(d) * b1 + p2(d) * b2 + p3(d) * b3;
  // p0, p1, p2, p3 are vertices of tetrahedron
  // b0, b1, b2, b3 are Barycentric coordinates of the tetrahedron center

  return center;
}

Point
circumcenter2D(const Point & p0, const Point & p1, const Point & p2)
{
  // Square of triangle edge lengths
  Real edge01 = (p0 - p1).norm_sq();
  Real edge02 = (p0 - p2).norm_sq();
  Real edge12 = (p1 - p2).norm_sq();

  // Barycentric weights for circumcenter
  Real weight0 = edge12 * (edge01 + edge02 - edge12);
  Real weight1 = edge02 * (edge01 + edge12 - edge02);
  Real weight2 = edge01 * (edge02 + edge12 - edge01);

  Real sum_weights = weight0 + weight1 + weight2;

  // Check to make sure vertices are not collinear
  if (MooseUtils::isZero(sum_weights))
    mooseError("Cannot evaluate circumcenter. Points should be non-collinear.");

  Real inv_sum_weights = 1.0 / sum_weights;

  // Barycentric coordinates
  Real b0 = weight0 * inv_sum_weights;
  Real b1 = weight1 * inv_sum_weights;
  Real b2 = weight2 * inv_sum_weights;

  return MathUtils::barycentricToCartesian2D(p0, p1, p2, b0, b1, b2);
}

Point
circumcenter3D(const Point & p0, const Point & p1, const Point & p2, const Point & p3)
{
  // Square of tetrahedron edge lengths
  Real edge01 = (p0 - p1).norm_sq();
  Real edge02 = (p0 - p2).norm_sq();
  Real edge03 = (p0 - p3).norm_sq();
  Real edge12 = (p1 - p2).norm_sq();
  Real edge13 = (p1 - p3).norm_sq();
  Real edge23 = (p2 - p3).norm_sq();

  // Barycentric weights for circumcenter
  Real weight0 = -2 * edge12 * edge13 * edge23 + edge01 * edge23 * (edge13 + edge12 - edge23) +
                 edge02 * edge13 * (edge12 + edge23 - edge13) +
                 edge03 * edge12 * (edge13 + edge23 - edge12);
  Real weight1 = -2 * edge02 * edge03 * edge23 + edge01 * edge23 * (edge02 + edge03 - edge23) +
                 edge13 * edge02 * (edge03 + edge23 - edge02) +
                 edge12 * edge03 * (edge02 + edge23 - edge03);
  Real weight2 = -2 * edge01 * edge03 * edge13 + edge02 * edge13 * (edge01 + edge03 - edge13) +
                 edge23 * edge01 * (edge03 + edge13 - edge01) +
                 edge12 * edge03 * (edge01 + edge13 - edge03);
  Real weight3 = -2 * edge01 * edge02 * edge12 + edge03 * edge12 * (edge01 + edge02 - edge12) +
                 edge23 * edge01 * (edge02 + edge12 - edge01) +
                 edge13 * edge02 * (edge01 + edge12 - edge02);

  Real sum_weights = weight0 + weight1 + weight2 + weight3;

  // Check to make sure vertices are not coplanar
  if (MooseUtils::isZero(sum_weights))
    mooseError("Cannot evaluate circumcenter. Points should be non-coplanar.");

  Real inv_sum_weights = 1.0 / sum_weights;

  // Barycentric coordinates
  Real b0 = weight0 * inv_sum_weights;
  Real b1 = weight1 * inv_sum_weights;
  Real b2 = weight2 * inv_sum_weights;
  Real b3 = weight3 * inv_sum_weights;

  return MathUtils::barycentricToCartesian3D(p0, p1, p2, p3, b0, b1, b2, b3);
}

} // namespace MathUtils

std::vector<std::vector<unsigned int>>
multiIndexHelper(unsigned int N, unsigned int K)
{
  std::vector<std::vector<unsigned int>> n_choose_k;
  std::vector<unsigned int> row;
  std::string bitmask(K, 1); // K leading 1's
  bitmask.resize(N, 0);      // N-K trailing 0's

  do
  {
    row.clear();
    row.push_back(0);
    for (unsigned int i = 0; i < N; ++i) // [0..N-1] integers
      if (bitmask[i])
        row.push_back(i + 1);
    row.push_back(N + 1);
    n_choose_k.push_back(row);
  } while (std::prev_permutation(bitmask.begin(), bitmask.end()));

  return n_choose_k;
}
