//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeStrainBase.h"

/**
 * ADComputeGreenLagrangeStrain defines a non-linear Green-Lagrange strain tensor
 */
class ADComputeGreenLagrangeStrain : public ADComputeStrainBase
{
public:
  static InputParameters validParams();

  ADComputeGreenLagrangeStrain(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;
};
