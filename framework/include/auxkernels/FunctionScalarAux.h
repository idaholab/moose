//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTIONSCALARAUX_H
#define FUNCTIONSCALARAUX_H

#include "AuxScalarKernel.h"

class FunctionScalarAux;
class Function;

template <>
InputParameters validParams<FunctionScalarAux>();

/**
 * Sets a value of a scalar variable based on the function
 */
class FunctionScalarAux : public AuxScalarKernel
{
public:
  FunctionScalarAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  std::vector<Function *> _functions;
};

#endif /* FUNCTIONSCALARAUX_H */
