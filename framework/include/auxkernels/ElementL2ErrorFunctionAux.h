//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTL2ERRORFUNCTIONAUX_H
#define ELEMENTL2ERRORFUNCTIONAUX_H

// MOOSE includes
#include "ElementLpNormAux.h"

// Forward declarations
class ElementL2ErrorFunctionAux;

template <>
InputParameters validParams<ElementL2ErrorFunctionAux>();

/**
 * A class for computing the element-wise L^2 error (actually L^p
 * error, if you set the value of p to something other than 2) of the
 * difference between an exact solution (typically represented by a
 * ParsedFunction) and the coupled solution variable.  The base
 * class implements the compute() function.
 */
class ElementL2ErrorFunctionAux : public ElementLpNormAux
{
public:
  /**
   * Class constructor
   * @param name Object name
   * @param parameters Object input parameters
   */
  ElementL2ErrorFunctionAux(const InputParameters & parameters);

protected:
  /**
   * Returns the difference between the solution variable and the
   * exact solution Function.
   */
  virtual Real computeValue() override;

  /// Function representing the exact solution.
  Function & _func;
};

#endif // ELEMENTL2ERRORFUNCTIONAUX_H
