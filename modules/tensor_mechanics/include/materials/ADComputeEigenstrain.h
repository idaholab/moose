//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADADCOMPUTEEIGENSTRAIN_H
#define ADADCOMPUTEEIGENSTRAIN_H

#include "ADComputeEigenstrainBase.h"

#define usingADComputeEigenstrainMembers usingComputeEigenstrainBaseMembers

template <ComputeStage>
class ADComputeEigenstrain;

declareADValidParams(ADComputeEigenstrain);

/**
 * ComputeEigenstrain computes an Eigenstrain that is a function of a single variable defined by a
 * base tensor and a scalar function defined in a Derivative Material.
 */
template <ComputeStage compute_stage>
class ADComputeEigenstrain : public ADComputeEigenstrainBase<compute_stage>
{
public:
  ADComputeEigenstrain(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;

  const ADMaterialProperty(Real) & _prefactor;

  RankTwoTensor _eigen_base_tensor;

  usingComputeEigenstrainBaseMembers;
};

#endif // ADADCOMPUTEEIGENSTRAIN_H
