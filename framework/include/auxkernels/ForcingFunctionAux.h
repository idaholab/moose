//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionAux.h"

/**
 * Add a forcing function to the previous time step value of an AuxVariable
 */
class ForcingFunctionAux : public FunctionAux
{
public:
  static InputParameters validParams();

  ForcingFunctionAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// AuxVariable value at previous time step
  const VariableValue & _u_old;
};
