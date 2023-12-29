//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

// Forward Declarations

/**
 * Accumulate values from one auxiliary variable into another
 */
class AccumulateAux : public AuxKernel
{
public:
  static InputParameters validParams();

  AccumulateAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// coupled variable values to be aggregated
  const std::vector<const VariableValue *> _values;

  /// The old variable value
  const VariableValue & _u_old;
};
