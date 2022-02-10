//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADHeatConductionTimeDerivative.h"
#include "RZSymmetry.h"

/**
 * Time derivative kernel used by heat conduction equation in arbitrary RZ symmetry
 */
class ADHeatConductionTimeDerivativeRZ : public ADHeatConductionTimeDerivative, public RZSymmetry
{
public:
  ADHeatConductionTimeDerivativeRZ(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual();

public:
  static InputParameters validParams();
};
