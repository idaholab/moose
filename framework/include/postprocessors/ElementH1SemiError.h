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
