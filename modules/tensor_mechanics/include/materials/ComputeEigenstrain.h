/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEEIGENSTRAIN_H
#define COMPUTEEIGENSTRAIN_H

#include "ComputeEigenstrainBase.h"

#include "RankTwoTensor.h"

/**
 * ComputeEigenstrain computes an Eigenstrain that is a function of a single variable defined by a
 * base tensor and a scalar function defined in a Derivative Material.
 */
class ComputeEigenstrain : public ComputeEigenstrainBase
{
public:
  ComputeEigenstrain(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain();

  const MaterialProperty<Real> & _prefactor;

  RankTwoTensor _eigen_base_tensor;
};

#endif // COMPUTEEIGENSTRAIN_H
