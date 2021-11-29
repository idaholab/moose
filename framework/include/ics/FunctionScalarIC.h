//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarInitialCondition.h"

class Function;

class FunctionScalarIC : public ScalarInitialCondition
{
public:
  static InputParameters validParams();

  FunctionScalarIC(const InputParameters & parameters);

protected:
  virtual Real value() override;

  unsigned int _ncomp;
  std::vector<const Function *> _func;
};
