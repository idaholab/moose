//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayKernel.h"

class ArrayDiffusion : public ArrayKernel
{
public:
  static InputParameters validParams();

  ArrayDiffusion(const InputParameters & parameters);

protected:
  virtual void initQpResidual() override;
  virtual void computeQpResidual(RealEigenVector & residual) override;
  virtual RealEigenVector computeQpJacobian() override;
  virtual RealEigenMatrix computeQpOffDiagJacobian(const MooseVariableFEBase & jvar) override;

  /// scalar diffusion coefficient
  const MaterialProperty<Real> * const _d;
  /// array diffusion coefficient
  const MaterialProperty<RealEigenVector> * const _d_array;
  /// matrix diffusion coefficient
  const MaterialProperty<RealEigenMatrix> * const _d_2d_array;
};
