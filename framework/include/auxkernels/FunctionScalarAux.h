//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxScalarKernel.h"

class Function;

/**
 * Sets a value of a scalar variable based on the function
 */
class FunctionScalarAux : public AuxScalarKernel
{
public:
  static InputParameters validParams();

  FunctionScalarAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  std::vector<const Function *> _functions;
};
