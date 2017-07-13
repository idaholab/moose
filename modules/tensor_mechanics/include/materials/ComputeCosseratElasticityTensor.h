/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECOSSERATELASTICITYTENSOR_H
#define COMPUTECOSSERATELASTICITYTENSOR_H

#include "ComputeElasticityTensorBase.h"

/**
 * ComputeElasticityTensor defines an elasticity tensor material for isi.
 */
class ComputeCosseratElasticityTensor : public ComputeElasticityTensorBase
{
public:
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

#endif // COMPUTECOSSERATELASTICITYTENSOR_H
