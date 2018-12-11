//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TENSORMECHADMATTEST_H
#define TENSORMECHADMATTEST_H

#include "ADMaterial.h"
#include "RankFourTensor.h"

template <ComputeStage>
class TensorMechADMatTest;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;

declareADValidParams(TensorMechADMatTest);

/**
 * TensorMechADMatTest is the base class for stress tensors
 */
template <ComputeStage compute_stage>
class TensorMechADMatTest : public ADMaterial<compute_stage>
{
public:
  TensorMechADMatTest(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  MaterialProperty<RankFourTensor> & _elasticity_tensor;
  ADMaterialProperty(RankTwoTensor) & _total_strain;
  ADMaterialProperty(RankTwoTensor) & _mechanical_strain;
  ADMaterialProperty(RankTwoTensor) & _stress;

  unsigned int _ndisp;
  std::vector<const ADVariableGradient *> _grad_disp;

  RankFourTensor _Cijkl;

  usingMaterialMembers;
};

#endif // TENSORMECHADMATTEST_H
