/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECOSSERATSTRESSBASE_H
#define COMPUTECOSSERATSTRESSBASE_H

#include "ComputeStressBase.h"

/**
 * ComputeCosseratStressBase is the base class for stress tensors
 */
class ComputeCosseratStressBase : public ComputeStressBase
{
public:
  ComputeCosseratStressBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpStress() = 0;

  /// The Cosserat curvature strain
  const MaterialProperty<RankTwoTensor> & _curvature;

  /// The Cosserat elastic flexural rigidity tensor
  const MaterialProperty<RankFourTensor> & _elastic_flexural_rigidity_tensor;

  /// the Cosserat couple-stress
  MaterialProperty<RankTwoTensor> & _stress_couple;

  /// derivative of couple-stress w.r.t. curvature
  MaterialProperty<RankFourTensor> & _Jacobian_mult_couple;
};

#endif // COMPUTECOSSERATSTRESSBASE_H
