//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseEnum.h"
#include "MooseTypes.h"
#include "ADReal.h"

#include <array>

namespace Moose
{
namespace Contact
{

CreateMooseEnumClass(FrictionCoefficientRegularization, NONE, ARCTAN_SLIP);

/// Orthonormal material frame ordered as first tangent, second tangent, and normal.
using ContactTangentialFrame = std::array<RealVectorValue, 3>;

struct ElasticSlipReturn
{
  /// Tangential LM vector, aligned with relative slip; secondary physical traction is its negative.
  ADRealVectorValue multiplier;
  RealVectorValue elastic_gap;
};

MooseEnum frictionCoefficientRegularizationOptions();

ContactTangentialFrame buildContactTangentialFrame(const Elem & elem,
                                                   const Point & reference_point,
                                                   const RealVectorValue & preferred_normal);

ADReal tangentialSlipMagnitude(const ADRealVectorValue & slip);

ElasticSlipReturn elasticSlipReturnMap(const ADRealVectorValue & trial_elastic_gap,
                                       const ADReal & friction_coefficient,
                                       const ADReal & contact_pressure,
                                       Real elastic_slip);

ADReal
arctanFrictionCoefficient(const ADReal & mu, const ADReal & slip_increment, Real reference_slip);

} // namespace Contact
} // namespace Moose
