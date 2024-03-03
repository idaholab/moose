//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADCompute2DSmallStrain.h"

/**
 * ADComputeAxisymmetricRZSmallStrain defines small strains in an Axisymmetric system.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ADComputeAxisymmetricRZSmallStrain : public ADCompute2DSmallStrain
{
public:
  static InputParameters validParams();

  ADComputeAxisymmetricRZSmallStrain(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual ADReal computeOutOfPlaneStrain() override;
};
