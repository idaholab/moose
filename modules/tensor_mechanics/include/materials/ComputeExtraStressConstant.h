/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEEXTRASTRESSCONSTANT_H
#define COMPUTEEXTRASTRESSCONSTANT_H

#include "ComputeExtraStressBase.h"

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
