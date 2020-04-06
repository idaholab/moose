//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrainBase.h"
#include "CompositeTensorBase.h"
#include "RankTwoTensor.h"

/**
 * CompositeEigenstrain provides a simple RankTwoTensor type
 * MaterialProperty that can be used as an Eigenstrain tensor in a mechanics simulation.
 * This tensor is computes as a weighted sum of base Eigenstrain tensors where each weight
 * can be a scalar material property that may depend on simulation variables.
 * The generic logic that computes a weighted sum of tensors is located in the
 * templated base class CompositeTensorBase.
 */
class CompositeEigenstrain : public CompositeTensorBase<RankTwoTensor, ComputeEigenstrainBase>
{
public:
  static InputParameters validParams();

  CompositeEigenstrain(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain();

  const std::string _M_name;
};
