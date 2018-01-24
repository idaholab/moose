//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PHASENORMALTENSOR_H
#define PHASENORMALTENSOR_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"
#include "RankTwoTensor.h"

class PhaseNormalTensor;

template <>
InputParameters validParams<PhaseNormalTensor>();

/**
 * Calculate phase normal tensor based on gradient
 */
class PhaseNormalTensor : public DerivativeMaterialInterface<Material>
{
public:
  PhaseNormalTensor(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  const VariableGradient & _grad_u;
  MaterialProperty<RankTwoTensor> & _normal_tensor;
};

#endif
