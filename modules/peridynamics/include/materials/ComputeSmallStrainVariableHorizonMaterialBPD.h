//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeSmallStrainMaterialBaseBPD.h"

/**
 * Material class for bond based peridynamic solid mechanics model based on irregular spatial
 * discretization
 */
class ComputeSmallStrainVariableHorizonMaterialBPD : public ComputeSmallStrainMaterialBaseBPD
{
public:
  static InputParameters validParams();

  ComputeSmallStrainVariableHorizonMaterialBPD(const InputParameters & parameters);

protected:
  virtual void computePeridynamicsParams() override;
};
