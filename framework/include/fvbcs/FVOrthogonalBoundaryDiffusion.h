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

class Function;

class FVOrthogonalBoundaryDiffusion : public FVFluxBC
{
public:
  FVOrthogonalBoundaryDiffusion(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADReal computeQpResidual() override;

  const Function & _function;
  const ADMaterialProperty<Real> & _coeff_elem;
  const ADMaterialProperty<Real> & _coeff_neighbor;
  const MooseArray<ADReal> & _diff_quant_elem;
  const MooseArray<ADReal> & _diff_quant_neighbor;
};
