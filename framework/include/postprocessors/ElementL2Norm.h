//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTL2NORM_H
#define ELEMENTL2NORM_H

#include "ElementIntegralVariablePostprocessor.h"

// Forward Declarations
class ElementL2Norm;

template <>
InputParameters validParams<ElementL2Norm>();

class ElementL2Norm : public ElementIntegralVariablePostprocessor
{
public:
  ElementL2Norm(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeQpIntegral() override;
};

#endif // ELEMENTL2NORM_H
