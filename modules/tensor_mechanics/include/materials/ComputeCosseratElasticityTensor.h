//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeElasticityTensorBase.h"

/**
 * ComputeElasticityTensor defines an elasticity tensor material for isi.
 */
class ComputeCosseratElasticityTensor : public ComputeElasticityTensorBase
{
public:
  static InputParameters validParams();

  ComputeCosseratElasticityTensor(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor();

  /// Conventional elasticity tensor
  RankFourTensor _Eijkl;

  /// Flexural rigidity tensor
  RankFourTensor _Bijkl;

  /// Flexural rigidity tensor at the qps
  MaterialProperty<RankFourTensor> & _elastic_flexural_rigidity_tensor;
};
