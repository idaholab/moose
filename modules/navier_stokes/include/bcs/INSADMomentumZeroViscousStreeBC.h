//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSADMomentumImplicitStressBC.h"
#include "MooseEnum.h"

/**
 * This class implements a zero normal viscous stress boundary condition
 */
class INSADMomentumZeroViscousStreeBC : public INSADMomentumImplicitStressBC
{
public:
  static InputParameters validParams();

  INSADMomentumZeroViscousStreeBC(const InputParameters & parameters);

protected:
  virtual ADReal viscousStress() override;
};
