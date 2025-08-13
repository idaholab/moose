//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"

#include "nlohmann/json.h"

#include "libmesh/libmesh_common.h"

#include "Eigen/Core"

#include <memory>

class MooseApp;

namespace libMesh
{
class Point;
template <typename T>
class DenseVector;
template <typename T>
class DenseMatrix;
template <typename T>
class NumericVector;
}

// Overloads for to_json, which _must_ be overloaded in the namespace
// in which the object is found in order to enable argument-dependent lookup.
// See https://en.cppreference.com/w/cpp/language/adl for more information
void to_json(nlohmann::json & json, const MooseApp & app); // MooseDocs:to_json

namespace libMesh
{
void to_json(nlohmann::json & json, const Point & p);
void to_json(nlohmann::json & json, const DenseVector<Real> & vector);
void to_json(nlohmann::json & json, const DenseMatrix<Real> & matrix);
void to_json(nlohmann::json & json, const std::unique_ptr<NumericVector<Number>> & vector);
}

namespace Eigen
{
template <typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
void to_json(nlohmann::json & json,
             const Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> & matrix);
}

namespace nlohmann
{
template <typename T>
struct adl_serializer<std::unique_ptr<T>>
{
  /// Serializer that will output a unique ptr if it exists. We wrap this
  /// with is_constructible_v so that we don't specialize types that
  /// don't already have a specialization. This is required for some earlier
  /// compilers, even though we're not using it at the moment
  static void to_json(json & j, const std::unique_ptr<T> & v)
  {
    if constexpr (std::is_constructible_v<nlohmann::json, T>)
    {
      if (v)
        j = *v;
      else
        j = nullptr;
    }
    else
      mooseAssert(false, "Should not get to this");
  }
};
}

namespace Eigen
{
template <typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
void
to_json(nlohmann::json & json, const Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> & matrix)
{
  if constexpr (Rows == 1 || Cols == 1)
  {
    std::vector<Scalar> values(matrix.data(), matrix.data() + matrix.rows() * matrix.cols());
    nlohmann::to_json(json, values);
  }
  else
  {
    const auto nrows = matrix.rows();
    const auto ncols = matrix.cols();
    std::vector<std::vector<Scalar>> values(nrows, std::vector<Scalar>(ncols));
    for (const auto i : make_range(nrows))
      for (const auto j : make_range(ncols))
        values[i][j] = matrix(i, j);
    nlohmann::to_json(json, values);
  }
}
}
