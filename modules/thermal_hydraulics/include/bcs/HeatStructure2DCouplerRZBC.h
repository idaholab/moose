//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatStructure2DCouplerBC.h"
#include "RZSymmetry.h"

/**
 * Applies BC for HeatStructure2DCoupler for cylindrical heat structure
 */
class HeatStructure2DCouplerRZBC : public HeatStructure2DCouplerBC, public RZSymmetry
{
public:
  HeatStructure2DCouplerRZBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

public:
  static InputParameters validParams();
};
