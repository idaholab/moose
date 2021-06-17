//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInterfaceKernel.h"

class FVOneVarDiffusionInterface : public FVInterfaceKernel
{
public:
  static InputParameters validParams();
  FVOneVarDiffusionInterface(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _coeff1_elem;
  const ADMaterialProperty<Real> & _coeff2_elem;
  const ADMaterialProperty<Real> & _coeff1_neighbor;
  const ADMaterialProperty<Real> & _coeff2_neighbor;
};
