//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

/**
 * Computes mass float rate from specified mass flux and cross-sectional area
 */
class SCMMassFlowRateIC : public InitialCondition
{
public:
  static InputParameters validParams();

  SCMMassFlowRateIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// Specified mass flux
  const Real & _mass_flux;
  /// Cross-sectional area
  const VariableValue & _area;
};
