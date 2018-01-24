/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTELAYEREDCOSSERATELASTICITYTENSOR_H
#define COMPUTELAYEREDCOSSERATELASTICITYTENSOR_H

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

#endif // COMPUTELAYEREDCOSSERATELASTICITYTENSOR_H
