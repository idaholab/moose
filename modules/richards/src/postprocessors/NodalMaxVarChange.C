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

#include "NodalMaxVarChange.h"

#include <algorithm> // std::max
#include <limits>
#include <cmath> // std::abs

template<>
InputParameters validParams<NodalMaxVarChange>()
{
  InputParameters params = validParams<NodalVariablePostprocessor>();
  params.addClassDescription("This postprocessor returns the value max(abs(variable - variable_old)) for the specified variable.");
  return params;
}

NodalMaxVarChange::NodalMaxVarChange(const std::string & name, InputParameters parameters) :
  NodalVariablePostprocessor(name, parameters),
  _u_old(_var.nodalSlnOld()),
  _value(-std::numeric_limits<Real>::max())
{
}

void
NodalMaxVarChange::initialize()
{
  _value = -std::numeric_limits<Real>::max();
}

void
NodalMaxVarChange::execute()
{
  _value = std::max(_value, std::abs(_u[_qp] - _u_old[_qp]));
}

Real
NodalMaxVarChange::getValue()
{
  gatherMax(_value);
  return _value;
}

void
NodalMaxVarChange::threadJoin(const UserObject & y)
{
  const NodalMaxVarChange & pps = static_cast<const NodalMaxVarChange &>(y);
  _value = std::max(_value, pps._value);
}
