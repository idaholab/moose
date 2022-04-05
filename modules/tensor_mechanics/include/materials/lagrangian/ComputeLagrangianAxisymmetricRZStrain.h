//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangian2DStrain.h"

/**
 * ComputeLagrangianAxisymmetricRZStrain defines the strain in RZ coordinates assuming axisymmetry
 * along the Z-axis, i.e. the displacements are independent of \theta.
 */
class ComputeLagrangianAxisymmetricRZStrain : public ComputeLagrangian2DStrain
{
public:
  static InputParameters validParams();

  ComputeLagrangianAxisymmetricRZStrain(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual Real computeOutOfPlaneGradDisp() override;
};
