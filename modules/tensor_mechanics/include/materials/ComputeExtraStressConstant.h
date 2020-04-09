//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeExtraStressBase.h"

/**
 * Computes a constant extra stress that is added to the stress calculated
 * by the constitutive model
 */
class ComputeExtraStressConstant : public ComputeExtraStressBase
{
public:
  static InputParameters validParams();

  ComputeExtraStressConstant(const InputParameters & parameters);

protected:
  virtual void computeQpExtraStress();

  const MaterialProperty<Real> & _prefactor;

  RankTwoTensor _extra_stress_tensor;
};
