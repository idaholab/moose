//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

/**
 * Implicit boundary conditions for Euler equations. Used for modeling
 * free in/outflow boundary conditions
 */
class CNSFVImplicitAdvectionBC : public FVFluxBC
{
public:
  CNSFVImplicitAdvectionBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  /// porosity
  // const VariableValue & _eps;
  const Real _eps;

  /// velocity
  const ADMaterialProperty<RealVectorValue> & _velocity;

  /// The advected quantity
  const MooseArray<ADReal> * _adv_quant;
};
