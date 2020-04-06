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
 * Calculate phase normal tensor based on gradient
 */
class PhaseNormalTensor : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  PhaseNormalTensor(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  const VariableGradient & _grad_u;
  MaterialProperty<RankTwoTensor> & _normal_tensor;
};
