//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"

class CNSFVMomPressure : public FVFluxKernel
{
public:
  static InputParameters validParams();
  CNSFVMomPressure(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// porosity
  // const VariableValue & _eps;
  const Real _eps;

  const ADMaterialProperty<RealVectorValue> & _vel_elem;
  const ADMaterialProperty<RealVectorValue> & _vel_neighbor;

  /// The pressure on the elem
  const ADMaterialProperty<Real> & _pressure_elem;

  /// The pressure on the neighbor
  const ADMaterialProperty<Real> & _pressure_neighbor;

  /// index x|y|z
  unsigned int _index;
};
