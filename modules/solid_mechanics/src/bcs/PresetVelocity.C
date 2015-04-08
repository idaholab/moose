/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PresetVelocity.h"
#include "Function.h"

template<>
InputParameters validParams<PresetVelocity>()
{
  InputParameters p = validParams<NodalBC>();
  p.addParam<Real>("velocity", 1, "Value of the velocity.  Used as scale factor if function is given.");
  p.addParam<FunctionName>("function", "", "Function describing the velocity.");
  return p;
}


PresetVelocity::PresetVelocity(const std::string & name, InputParameters parameters) :
  PresetNodalBC(name, parameters),
  _u_old(valueOld()),
  _velocity(parameters.get<Real>("velocity")),
  _function(parameters.get<FunctionName>("function") != "" ? &getFunction("function") : NULL)
{
}

Real
PresetVelocity::computeQpValue()
{
  Real vel(_velocity);
  if (_function)
  {
    Real v2( _function->value(_t, *_current_node) );
    Real v1( _function->value(_t-_dt, *_current_node) );
    vel *= 0.5*(v1 + v2);
  }

  return _u_old[_qp] + _dt * vel;
}
