//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADArrayKernel.h"

/**
 * Array kernel for adding a reaction term
 */
class ADArrayReaction : public ADArrayKernel
{
public:
  static InputParameters validParams();

  ADArrayReaction(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(ADRealEigenVector & residual) override;

  /// scalar diffusion coefficient
  const MaterialProperty<Real> * _r;
  /// array diffusion coefficient
  const MaterialProperty<RealEigenVector> * const _r_array;
  /// matrix diffusion coefficient (notably for anisotropic diffusion)
  const MaterialProperty<RealEigenMatrix> * const _r_2d_array;
};
