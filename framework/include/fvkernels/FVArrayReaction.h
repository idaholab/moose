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

class FVArrayReaction : public FVArrayElementalKernel
{
public:
  static InputParameters validParams();
  FVArrayReaction(const InputParameters & parameters);

protected:
  ADRealEigenVector computeQpResidual() override;

  /// scalar diffusion coefficient
  const ADMaterialProperty<Real> * const _r;
  /// array diffusion coefficient
  const ADMaterialProperty<RealEigenVector> * const _r_array;
  /// matrix diffusion coefficient
  const ADMaterialProperty<RealEigenMatrix> * const _r_2d_array;
};
