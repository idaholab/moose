//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayDGKernel.h"

/**
 * Array version of DGDiffusion
 */
class ArrayDGDiffusion : public ArrayDGKernel
{
public:
  static InputParameters validParams();

  ArrayDGDiffusion(const InputParameters & parameters);

protected:
  virtual void initQpResidual(Moose::DGResidualType type) override;
  virtual void computeQpResidual(Moose::DGResidualType type, RealEigenVector & residual) override;
  virtual RealEigenVector computeQpJacobian(Moose::DGJacobianType type) override;

  Real _epsilon;
  Real _sigma;
  const MaterialProperty<RealEigenVector> & _diff;
  const MaterialProperty<RealEigenVector> & _diff_neighbor;

  RealEigenVector _res1;
  RealEigenVector _res2;
};
