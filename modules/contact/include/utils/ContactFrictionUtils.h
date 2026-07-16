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
#include "MooseEnum.h"
#include "MooseTypes.h"

#include "libmesh/libmesh.h"

#include <cmath>

namespace Moose
{
namespace Contact
{

CreateMooseEnumClass(FrictionCoefficientRegularization, NONE, ARCTAN_SLIP);

inline MooseEnum
frictionCoefficientRegularizationOptions()
{
  MooseEnum options(getFrictionCoefficientRegularizationOptions(), "NONE");
  options.addDocumentation("NONE", "Use the supplied Coulomb friction coefficient.");
  options.addDocumentation(
      "ARCTAN_SLIP",
      "Scale the Coulomb friction coefficient by an arctangent function of the slip increment.");
  return options;
}

template <typename TMu, typename TSlip>
auto
regularizedFrictionCoefficient(const TMu & mu,
                               const TSlip & slip_increment,
                               const FrictionCoefficientRegularization regularization,
                               const Real reference_slip) -> decltype(mu + mu * slip_increment)
{
  using std::atan;
  using Result = decltype(mu + mu * slip_increment);

  if (regularization == FrictionCoefficientRegularization::NONE)
    return Result(mu);

  mooseAssert(reference_slip > 0.0,
              "Friction coefficient regularization requires a positive reference slip");

  return mu * (2.0 / libMesh::pi) * atan(slip_increment / reference_slip);
}

} // namespace Contact
} // namespace Moose
