//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatConduction.h"
#include "RZSymmetry.h"

/**
 * Heat conduction kernel in arbitrary RZ symmetry
 */
class HeatConductionRZ : public HeatConductionKernel, public RZSymmetry
{
public:
  HeatConductionRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

public:
  static InputParameters validParams();
};
