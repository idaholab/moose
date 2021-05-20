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

class PCNSFVMomentumPressureFlux : public FVFluxKernel
{
public:
  static InputParameters validParams();
  PCNSFVMomentumPressureFlux(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// The porosity on the element
  const MaterialProperty<Real> & _eps_elem;
  /// The porosity on the neighbor
  const MaterialProperty<Real> & _eps_neighbor;
  /// The pressure on the element
  const ADMaterialProperty<Real> & _p_elem;
  /// The pressure on the neighbor
  const ADMaterialProperty<Real> & _p_neighbor;
  /// index x|y|z
  const unsigned int _index;
};
