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

#ifndef ELEMENTH1ERRORFUNCTIONAUX_H
#define ELEMENTH1ERRORFUNCTIONAUX_H

// MOOSE includes
#include "ElementL2ErrorFunctionAux.h"

// Forward declarations
class ElementH1ErrorFunctionAux;

template <>
InputParameters validParams<ElementH1ErrorFunctionAux>();

/**
 * A class for computing the element-wise H1 error (actually W^{1,p}
 * error, if you set the value of p to something other than 2.0) of
 * the difference between an exact solution (typically represented by
 * a ParsedFunction) and the specified solution variable.
 */
class ElementH1ErrorFunctionAux : public ElementL2ErrorFunctionAux
{
public:
  /**
   * Class constructor
   * @param name Object name
   * @param parameters Object input parameters
   */
  ElementH1ErrorFunctionAux(const InputParameters & parameters);

  /**
   * Overrides ElementLpNormAux since we want to raise to a power
   * in computeValue() instead.
   */
  virtual void compute() override;

protected:
  /**
   * Computes the error at the current qp.
   */
  virtual Real computeValue() override;

  /**
   * The gradient of the computed solution.
   */
  const VariableGradient & _grad_coupled_var;
};

#endif // ELEMENTH1ERRORFUNCTIONAUX_H
