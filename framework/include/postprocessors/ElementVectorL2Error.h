//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTVECTORL2ERROR_H
#define ELEMENTVECTORL2ERROR_H

#include "ElementIntegralPostprocessor.h"

class Function;

// Forward Declarations
class ElementVectorL2Error;

template <>
InputParameters validParams<ElementVectorL2Error>();

class ElementVectorL2Error : public ElementIntegralPostprocessor
{
public:
  ElementVectorL2Error(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeQpIntegral() override;

  Function & _funcx;
  Function & _funcy;
  Function & _funcz;

  const VariableValue & _u; // FE solution in x
  const VariableValue & _v; // FE solution in y
  const VariableValue & _w; // FE solution in z
};

#endif // ELEMENTVECTORL2ERROR_H
