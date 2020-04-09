//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Compute2DSmallStrain.h"

/**
 * ComputeAxisymmetricRZSmallStrain defines small strains in an Axisymmetric system.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ComputeAxisymmetricRZSmallStrain : public Compute2DSmallStrain
{
public:
  static InputParameters validParams();

  ComputeAxisymmetricRZSmallStrain(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual Real computeOutOfPlaneStrain() override;
};
