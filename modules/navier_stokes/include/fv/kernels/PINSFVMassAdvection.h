//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVMomentumAdvection.h"

/**
 * A flux kernel transporting mass in porous media across cell faces
 */
class PINSFVMassAdvection : public INSFVMomentumAdvection
{
public:
  static InputParameters validParams();
  PINSFVMassAdvection(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the porosity
  const ADVariableValue & _eps;
  /// the neighbor element porosity
  const ADVariableValue & _eps_neighbor;
};
