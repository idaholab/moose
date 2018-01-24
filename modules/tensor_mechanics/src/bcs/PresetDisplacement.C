/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PresetDisplacement.h"
#include "Function.h"

template <>
InputParameters
validParams<PresetDisplacement>()
{
  InputParameters params = validParams<NodalBC>();
  params.addClassDescription(
      "Prescribe the displacement on a given boundary in a given direction.");
  params.addParam<Real>("scale_factor", 1, "Scale factor if function is given.");
  params.addParam<FunctionName>("function", "1", "Function describing the displacement.");
  params.addRequiredCoupledVar("velocity", "The velocity variable.");
  params.addRequiredCoupledVar("acceleration", "The acceleration variable.");
  params.addRequiredParam<Real>("beta", "beta parameter for Newmark time integration.");
  return params;
}

PresetDisplacement::PresetDisplacement(const InputParameters & parameters)
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
PresetDisplacement::computeQpValue()
{
  Point p;
  Real vel = _function.timeDerivative(_t, p);
  Real vel_old = _function.timeDerivative(_t - _dt, p);
  Real accel = (vel - vel_old) / _dt;

  return _u_old[_qp] + _dt * _vel_old[_qp] +
         ((0.5 - _beta) * _accel_old[_qp] + _beta * accel) * _dt * _dt;
}
