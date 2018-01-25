//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEEIGENSTRAIN_H
#define COMPUTEEIGENSTRAIN_H

#include "ComputeEigenstrainBase.h"

#include "RankTwoTensor.h"

class ComputeEigenstrain;

template <>
InputParameters validParams<ComputeEigenstrain>();

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
