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

class ArrayReaction : public ArrayKernel
{
public:
  static InputParameters validParams();

  ArrayReaction(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(RealEigenVector & residual) override;
  virtual RealEigenVector computeQpJacobian() override;
  virtual RealEigenMatrix computeQpOffDiagJacobian(const MooseVariableFEBase & jvar) override;

  /// scalar diffusion coefficient
  const MaterialProperty<Real> * const _r;
  /// array diffusion coefficient
  const MaterialProperty<RealEigenVector> * const _r_array;
  /// matrix diffusion coefficient
  const MaterialProperty<RealEigenMatrix> * const _r_2d_array;
};
