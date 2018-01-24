//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTIONSCALARIC_H
#define FUNCTIONSCALARIC_H

#include "ScalarInitialCondition.h"

// Forward Declarations
class FunctionScalarIC;
class Function;

template <>
InputParameters validParams<FunctionScalarIC>();

class FunctionScalarIC : public ScalarInitialCondition
{
public:
  FunctionScalarIC(const InputParameters & parameters);

protected:
  virtual Real value() override;

  unsigned int _ncomp;
  std::vector<Function *> _func;
};

#endif // FUNCTIONSCALARIC_H
