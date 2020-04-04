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

/**
 * CompositeMobilityTensor provides a simple RealTensorValue type
 * MaterialProperty that can be used as a mobility in a phase field simulation.
 * This mobility is computes as a weighted sum of base mobilities where each weight
 * can be a scalar material property that may depend on simulation variables.
 * The generic logic that computes a weighted sum of tensors is located in the
 * templated base class CompositeTensorBase.
 */
class CompositeMobilityTensor : public CompositeTensorBase<RealTensorValue, Material>
{
public:
  static InputParameters validParams();

  CompositeMobilityTensor(const InputParameters & parameters);

protected:
  void computeQpProperties();

  const std::string _M_name;
  MaterialProperty<RealTensorValue> & _M;
};
