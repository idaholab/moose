//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEEXTRASTRESSCONSTANT_H
#define COMPUTEEXTRASTRESSCONSTANT_H

#include "ComputeExtraStressBase.h"

class ComputeExtraStressConstant;

template <>
InputParameters validParams<ComputeExtraStressConstant>();

/**
 * ComputeEigenstrain computes an Eigenstrain that is a function of a single variable defined by a
 * base tensor and a scalar function defined in a Derivative Material.
 */
class ComputeExtraStressConstant : public ComputeExtraStressBase
{
public:
  ComputeExtraStressConstant(const InputParameters & parameters);

protected:
  virtual void computeQpExtraStress();

  const MaterialProperty<Real> & _prefactor;

  RankTwoTensor _extra_stress_tensor;
};

#endif // COMPUTEEXTRASTRESSCONSTANT_H
