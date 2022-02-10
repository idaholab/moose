//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADHeatConduction.h"
#include "RZSymmetry.h"

/**
 * Heat conduction kernel in arbitrary RZ symmetry
 */
class ADHeatConductionRZ : public ADHeatConduction, public RZSymmetry
{
public:
  ADHeatConductionRZ(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual();

public:
  static InputParameters validParams();
};
