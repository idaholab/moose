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

class NSFVMomentumPressureFluxKernel : public FVFluxKernel
{
public:
  static InputParameters validParams();
  NSFVMomentumPressureFluxKernel(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// The pressure on the element
  const ADMaterialProperty<Real> & _p_elem;
  /// The pressure on the neighbor
  const ADMaterialProperty<Real> & _p_neighbor;
  /// index x|y|z
  const unsigned int _index;
};
