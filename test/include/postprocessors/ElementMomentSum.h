//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTMOMENTSUM_H
#define ELEMENTMOMENTSUM_H

#include "ElementIntegralVariablePostprocessor.h"

// Forward Declarations
class ElementMomentSum;

template <>
InputParameters validParams<ElementMomentSum>();

class ElementMomentSum : public ElementIntegralVariablePostprocessor
{
public:
  ElementMomentSum(const InputParameters & parameters);

protected:
  virtual void execute() override;

  const DenseVector<Number> & _elemental_sln;
};

#endif // ELEMENTMOMENTSUM_H
