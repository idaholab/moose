/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

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
