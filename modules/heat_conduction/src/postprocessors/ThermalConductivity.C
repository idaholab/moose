/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ThermalConductivity.h"

template <>
InputParameters
validParams<ThermalConductivity>()
{
  InputParameters params = validParams<SideAverageValue>();
  params.addRequiredParam<Real>("dx", "Length between sides of sample in length_scale");
  params.addRequiredParam<PostprocessorName>(
      "flux", "Heat flux out of 'cold' boundary in solution units, should always be positive");
  params.addRequiredParam<PostprocessorName>("T_hot", "Temperature on 'hot' boundary in K");
  params.addParam<Real>("length_scale", 1e-8, "lengthscale of the solution, default is 1e-8");
  params.addParam<Real>("k0", 0.0, "Initial value of the thermal conductivity");
  return params;
}

ThermalConductivity::ThermalConductivity(const InputParameters & parameters)
  : SideAverageValue(parameters),
    _dx(getParam<Real>("dx")),
    _flux(getPostprocessorValue("flux")),
    _T_hot(getPostprocessorValue("T_hot")),
    _length_scale(getParam<Real>("length_scale")),
    _k0(getParam<Real>("k0")),
    _step_zero(declareRestartableData<bool>("step_zero", true))
{
}

Real
ThermalConductivity::getValue()
{
  const Real T_cold = SideAverageValue::getValue();
  Real Th_cond = 0.0;
  if (_t_step >= 1)
    _step_zero = false;

  // Calculate effective thermal conductivity in W/(length_scale-K)
  if (std::abs(_T_hot - T_cold) > 1.0e-20)
    Th_cond = std::abs(_flux) * _dx / std::abs(_T_hot - T_cold);

  if (_step_zero)
    return _k0;
  else
    return Th_cond / _length_scale; // In W/(m-K)
}
