//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTH1SEMIERROR_H
#define ELEMENTH1SEMIERROR_H

#include "ElementIntegralVariablePostprocessor.h"

class Function;

// Forward Declarations
class ElementH1SemiError;

template <>
InputParameters validParams<ElementH1SemiError>();

/**
 * This postprocessor will print out the h1 seminorm between the computed
 * solution and the passed function.
 * ||u-f||_{H^1} = sqrt( \int |grad u - grad f|^2 dx )
 */
class ElementH1SemiError : public ElementIntegralVariablePostprocessor
{
public:
  ElementH1SemiError(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeQpIntegral() override;
  Function & _func;
};

#endif // ELEMENTH1SEMIERROR_H
