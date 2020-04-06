//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThermalMaterialBaseBPD.h"

/**
 * Material class for bond based peridynamic heat conduction model based on regular spatial
 * discretization
 */
class ThermalConstantHorizonMaterialBPD : public ThermalMaterialBaseBPD
{
public:
  static InputParameters validParams();

  ThermalConstantHorizonMaterialBPD(const InputParameters & parameters);

protected:
  virtual void computePeridynamicsParams(const Real ave_thermal_conductivity) override;
};
