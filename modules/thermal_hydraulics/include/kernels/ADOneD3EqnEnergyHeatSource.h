//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 * Volumetric heat source for 1-phase flow channel
 */
class ADOneD3EqnEnergyHeatSource : public ADKernel
{
public:
  ADOneD3EqnEnergyHeatSource(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  /// Heat source function
  const Moose::Functor<ADReal> & _q;
  /// Cross sectional area
  const VariableValue & _A;

public:
  static InputParameters validParams();
};
