//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "CompositeTensorBase.h"
#include "RankFourTensor.h"

/**
 * CompositeElasticityTensor provides a simple RankFourTensor type
 * MaterialProperty that can be used as an Elasticity tensor in a mechanics simulation.
 * This tensor is computes as a weighted sum of base elasticity tensors where each weight
 * can be a scalar material property that may depend on simulation variables.
 * The generic logic that computes a weighted sum of tensors is located in the
 * templated base class CompositeTensorBase.
 */
class CompositeElasticityTensor : public CompositeTensorBase<RankFourTensor, Material>
{
public:
  static InputParameters validParams();

  CompositeElasticityTensor(const InputParameters & parameters);

protected:
  void computeQpProperties();

  /// Base name of the material system
  const std::string _base_name;
  const std::string _M_name;

  MaterialProperty<RankFourTensor> & _M;
};
