//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrainBase.h"

#include "RankTwoTensor.h"

/**
 * ComputeVariableBaseEigenstrain computes an Eigenstrain based on a real tensor value material
 * property base (a),
 * a real material property prefactor (p) and a rank two tensor offset tensor (b)
 * p * a + b
 */
class ComputeVariableBaseEigenStrain : public ComputeEigenstrainBase
{
public:
  static InputParameters validParams();

  ComputeVariableBaseEigenStrain(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain();

  const MaterialProperty<RealTensorValue> & _base_tensor;
  const MaterialProperty<Real> & _prefactor;
  RankTwoTensor _offset_tensor;
};
