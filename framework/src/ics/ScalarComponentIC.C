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

#include "ScalarComponentIC.h"
#include "MooseVariableScalar.h"

template <>
InputParameters
validParams<ScalarComponentIC>()
{
  InputParameters params = validParams<ScalarInitialCondition>();
  params.addRequiredParam<std::vector<Real>>("values",
                                             "Initial values to initialize the scalar variable.");

  return params;
}

ScalarComponentIC::ScalarComponentIC(const InputParameters & parameters)
  : ScalarInitialCondition(parameters), _initial_values(getParam<std::vector<Real>>("values"))
{
  if (_initial_values.size() < _var.order())
    mooseError("The initial vector values size given to the scalar variable '",
               name(),
               "' has wrong size.");
}

Real
ScalarComponentIC::value()
{
  return _initial_values[_i];
}
