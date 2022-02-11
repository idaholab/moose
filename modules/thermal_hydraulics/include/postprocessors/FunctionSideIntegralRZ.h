//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionSideIntegral.h"
#include "RZSymmetry.h"

/**
 * Integrates a function over sides for RZ geometry modeled by XY domain
 */
class FunctionSideIntegralRZ : public FunctionSideIntegral, public RZSymmetry
{
public:
  FunctionSideIntegralRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

public:
  static InputParameters validParams();
};
