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
 * ComputeLayeredCosseratElasticityTensor defines an
 * elasticity tensor and an elastic flexural rigidity
 * tensor for use in simulations with layered
 * Cosserat materials.  The layering direction is
 * assumed to be in the "z" direction.
 */
class ComputeLayeredCosseratElasticityTensor : public ComputeElasticityTensorBase
{
public:
  static InputParameters validParams();

  ComputeLayeredCosseratElasticityTensor(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor();

  /// Conventional elasticity tensor
  RankFourTensor _Eijkl;

  /// Flexural rigidity tensor
  RankFourTensor _Bijkl;

  /**
   * Inverse of elasticity tensor.
   * The usual _Eijkl.invSymm() cannot be used here as _Eijkl
   * does not possess the usual symmetries
   */
  RankFourTensor _Cijkl;

  /// Flexural rigidity tensor at the qps
  MaterialProperty<RankFourTensor> & _elastic_flexural_rigidity_tensor;

  /// Compliance tensor (_Eijkl^-1) at the qps
  MaterialProperty<RankFourTensor> & _compliance;
};
