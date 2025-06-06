//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * Adds an anisotropic friction linear or quadratic terms to a region, that can be turned
 * on or off using the Controls system
 */
class NSFVFrictionFlowDiodeFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  NSFVFrictionFlowDiodeFunctorMaterial(const InputParameters & parameters);

private:
  /// Direction of the diode
  const RealVectorValue _direction;

  /// Magnitude of the additional linear resistance
  const RealVectorValue _linear_resistance;

  /// Magnitude of the additional quadratic resistance
  const RealVectorValue _quadratic_resistance;

  /// Base linear friction coefficient, from the correlation for the porous media friction
  const Moose::Functor<ADRealVectorValue> & _base_linear_friction;

  /// Base quadratic friction coefficient, from the correlation for the porous media friction
  const Moose::Functor<ADRealVectorValue> & _base_quadratic_friction;

  /// Whether the diode is active or not
  const bool & _diode_on;
};
