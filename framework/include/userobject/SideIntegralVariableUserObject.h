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

#ifndef SIDEINTEGRALVARIABLEUSEROBJECT_H
#define SIDEINTEGRALVARIABLEUSEROBJECT_H

#include "SideIntegralUserObject.h"
#include "MooseVariableInterface.h"

// Forward Declarations
class SideIntegralVariableUserObject;

template <>
InputParameters validParams<SideIntegralVariableUserObject>();

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class SideIntegralVariableUserObject : public SideIntegralUserObject, public MooseVariableInterface
{
public:
  SideIntegralVariableUserObject(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;
  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;
};

#endif
