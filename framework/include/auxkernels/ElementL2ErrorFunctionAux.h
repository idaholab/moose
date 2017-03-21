/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
