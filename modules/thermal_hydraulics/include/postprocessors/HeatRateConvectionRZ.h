//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatRateConvection.h"
#include "RZSymmetry.h"

/**
 * Integrates a cylindrical heat structure boundary convective heat flux
 */
class HeatRateConvectionRZ : public HeatRateConvection, public RZSymmetry
{
public:
  HeatRateConvectionRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

public:
  static InputParameters validParams();
};
