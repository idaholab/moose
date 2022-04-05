//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStrain.h"

/**
 * ComputeLagrangian2DStrain defines wraps around ComputeLagrangianStrain to provide out-of-plane
 * strain(s). The out-of-plane displacement component(s) are assumed to be zero.
 */
class ComputeLagrangian2DStrain : public ComputeLagrangianStrain
{
public:
  static InputParameters validParams();

  ComputeLagrangian2DStrain(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void calculateDeformationGradient() override;

  /// Computes the current out-of-plane component of the displacement gradient
  virtual Real computeOutOfPlaneGradDisp() = 0;

  const unsigned int _out_of_plane_direction;
};
