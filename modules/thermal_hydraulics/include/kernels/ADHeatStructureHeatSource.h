//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"
#include "Function.h"

/**
 *
 */
class ADHeatStructureHeatSource : public ADKernel
{
public:
  ADHeatStructureHeatSource(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  const Real & _power_fraction;
  const VariableValue & _total_power;
  const Function & _power_shape_function;
  const PostprocessorValue & _power_shape_integral;
  const Real & _scale;
  const Real & _num_units;

public:
  static InputParameters validParams();
};
