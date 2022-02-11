//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADHeatStructureHeatSource.h"
#include "RZSymmetry.h"

/**
 * Forcing function used in the heat conduction equation in arbitrary RZ symmetry
 */
class ADHeatStructureHeatSourceRZ : public ADHeatStructureHeatSource, public RZSymmetry
{
public:
  ADHeatStructureHeatSourceRZ(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

public:
  static InputParameters validParams();
};
