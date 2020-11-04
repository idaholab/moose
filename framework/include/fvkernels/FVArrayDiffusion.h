//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVArrayFluxKernel.h"

class FVArrayDiffusion : public FVArrayFluxKernel
{
public:
  static InputParameters validParams();
  FVArrayDiffusion(const InputParameters & params);

protected:
  virtual ADRealEigenVector computeQpResidual() override;

  /// scalar diffusion coefficient
  const ADMaterialProperty<Real> * const _d_elem;
  /// array diffusion coefficient
  const ADMaterialProperty<RealEigenVector> * const _d_array_elem;
  /// matrix diffusion coefficient
  const ADMaterialProperty<RealEigenMatrix> * const _d_2d_array_elem;

  const ADMaterialProperty<Real> * const _d_neighbor;
  const ADMaterialProperty<RealEigenVector> * const _d_array_neighbor;
  const ADMaterialProperty<RealEigenMatrix> * const _d_2d_array_neighbor;
};
