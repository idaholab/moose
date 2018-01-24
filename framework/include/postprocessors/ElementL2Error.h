//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTL2ERROR_H
#define ELEMENTL2ERROR_H

#include "ElementIntegralVariablePostprocessor.h"

class Function;

// Forward Declarations
class ElementL2Error;

template <>
InputParameters validParams<ElementL2Error>();

class ElementL2Error : public ElementIntegralVariablePostprocessor
{
public:
  ElementL2Error(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeQpIntegral() override;

  Function & _func;
};

#endif // ELEMENTL2ERROR_H
