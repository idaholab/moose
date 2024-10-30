//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
