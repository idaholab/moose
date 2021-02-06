//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

/**
 * Imposes a Boussinesq force on the momentum equation. Useful for modeling natural convection
 * within an incompressible Navier-Stokes approximation
 */
class INSFVMomentumBoussinesq : public FVElementalKernel
{
public:
  static InputParameters validParams();
  INSFVMomentumBoussinesq(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  const ADVariableValue & _temperature;
  const RealVectorValue _gravity;
  /// The thermal expansion coefficient
  const ADMaterialProperty<Real> & _alpha;
  /// Reference temperature at which the value of _rho was measured
  const Real _ref_temperature;
  /// the density
  const Real & _rho;
  /// index x|y|z
  const unsigned int _index;
};
