/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PresetAcceleration.h"
#include "Function.h"

template <>
InputParameters
validParams<PresetAcceleration>()
{
  InputParameters params = validParams<NodalBC>();
  params.addClassDescription("Prescribe acceleration on a given boundary in a given direction");
  params.addParam<Real>("scale_factor", 1, "Scale factor if function is given.");
  params.addParam<FunctionName>("function", "1", "Function describing the velocity.");
  params.addRequiredCoupledVar("velocity", "The velocity variable.");
  params.addRequiredCoupledVar("acceleration", "The acceleration variable.");
  params.addRequiredParam<Real>("beta", "beta parameter for Newmark time integration.");
  return params;
}

PresetAcceleration::PresetAcceleration(const InputParameters & parameters)
  : PresetNodalBC(parameters),
    _u_old(valueOld()),
    _scale_factor(parameters.get<Real>("scale_factor")),
    _function(getFunction("function")),
    _vel_old(coupledValueOld("velocity")),
    _accel_old(coupledValueOld("acceleration")),
    _beta(getParam<Real>("beta"))
{
}

Real
PresetAcceleration::computeQpValue()
{
  Real accel = _function.value(_t, *_current_node);

  // Integrate acceleration using Newmark time integration to get displacement
  return _u_old[_qp] + _dt * _vel_old[_qp] +
         ((0.5 - _beta) * _accel_old[_qp] + _beta * accel) * _dt * _dt;
}
