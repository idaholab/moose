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
 * Implicit boundary conditions for the boundary pressure term from
 * the momentum equation
 */
class CNSFVMomImplicitPressureBC : public FVFluxBC
{
public:
  CNSFVMomImplicitPressureBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  /// porosity
  const MaterialProperty<Real> * const _eps;

  /// The pressure on the elem
  const ADMaterialProperty<Real> & _pressure;

  /// index x|y|z
  unsigned int _index;
};
