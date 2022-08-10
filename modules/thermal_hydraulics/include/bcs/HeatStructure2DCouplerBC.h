//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatStructure2DCouplerBCBase.h"

/**
 * Applies BC for HeatStructure2DCoupler for plate heat structure
 */
class HeatStructure2DCouplerBC : public HeatStructure2DCouplerBCBase
{
public:
  HeatStructure2DCouplerBC(const InputParameters & parameters);

  virtual ADReal computeQpResidual() override;

protected:
  /// Heat transfer coefficient
  const Function & _htc;

public:
  static InputParameters validParams();
};
