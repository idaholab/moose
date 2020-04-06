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
#include "DerivativeMaterialInterface.h"
#include "RankTwoTensor.h"

/**
 * Base class to define GB dependent properties
 */
class GBDependentTensorBase : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  GBDependentTensorBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() = 0;
  virtual void computeQpProperties() = 0;

  const VariableValue & _gb;
  Real _bulk_parameter;
  Real _gb_parameter;

  const MaterialProperty<RankTwoTensor> & _gb_normal_tensor;
  MaterialProperty<RealTensorValue> & _gb_dependent_tensor;
};
