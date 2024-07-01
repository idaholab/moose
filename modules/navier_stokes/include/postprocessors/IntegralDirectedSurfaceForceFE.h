//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "SideIntegralPostprocessor.h"
#include "MathFVUtils.h"

/**
 * Postprocessor which computes the directed force coming from friction and pressure
 * differences on a surface defined as:
 *
 * $F_d = \int\limits_S (\sigma \vec{n}) \cdot \vec{d} dS$
 *
 * where $\sigma$ is the Cauchy stress tensor
 */
class IntegralDirectedSurfaceForceFE : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  IntegralDirectedSurfaceForceFE(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// The dynamic viscosity
  const ADMaterialProperty<Real> & _mu;

  /// Pressure field
  const VariableValue & _pressure;

  /// The velocity
  const VectorVariableGradient & _grad_vel;

  /// The direction in which the force is measured
  const RealVectorValue _direction;
};
