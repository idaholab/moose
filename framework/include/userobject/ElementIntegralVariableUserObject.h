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

#ifndef ELEMENTINTEGRALVARIABLEUSEROBJECT_H
#define ELEMENTINTEGRALVARIABLEUSEROBJECT_H

#include "ElementIntegralUserObject.h"
#include "MooseVariableInterface.h"

//Forward Declarations
class ElementIntegralVariableUserObject;

template<>
InputParameters validParams<ElementIntegralVariableUserObject>();

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class ElementIntegralVariableUserObject :
  public ElementIntegralUserObject,
  public MooseVariableInterface
{
public:
  ElementIntegralVariableUserObject(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

  MooseVariable & _var;

  /// Holds the solution at current quadrature points
  VariableValue & _u;
  /// Holds the solution gradient at the current quadrature points
  VariableGradient & _grad_u;
};

#endif
