/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HomogenizedThermalConductivity.h"
#include "SubProblem.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<HomogenizedThermalConductivity>()
{
  InputParameters params = validParams<ElementAverageValue>();
  params.addRequiredCoupledVar("temp_x", "solution in x");
  params.addCoupledVar("temp_y", "solution in y");
  params.addCoupledVar("temp_z", "solution in z");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component",
      "component < 3",
      "An integer corresponding to the direction this pp acts in (0 for x, 1 for y, 2 for z)");
  params.addParam<Real>("scale_factor", 1, "Scale factor");
  params.addParam<MaterialPropertyName>(
      "diffusion_coefficient",
      "thermal_conductivity",
      "Property name of the diffusivity (Default: thermal_conductivity)");
  return params;
}

HomogenizedThermalConductivity::HomogenizedThermalConductivity(const InputParameters & parameters)
  : ElementAverageValue(parameters),
    _grad_temp_x(coupledGradient("temp_x")),
    _grad_temp_y(_subproblem.mesh().dimension() >= 2 ? coupledGradient("temp_y") : _grad_zero),
    _grad_temp_z(_subproblem.mesh().dimension() == 3 ? coupledGradient("temp_z") : _grad_zero),
    _component(getParam<unsigned int>("component")),
    _diffusion_coefficient(getMaterialProperty<Real>("diffusion_coefficient")),
    _scale(getParam<Real>("scale_factor"))
{
}

void
HomogenizedThermalConductivity::initialize()
{
  _integral_value = 0.0;
  _volume = 0.0;
}

void
HomogenizedThermalConductivity::execute()
{
  _integral_value += computeIntegral();
  _volume += _current_elem_volume;
}

Real
HomogenizedThermalConductivity::getValue()
{
  gatherSum(_integral_value);
  gatherSum(_volume);

  return (_integral_value / _volume);
}

void
HomogenizedThermalConductivity::threadJoin(const UserObject & y)
{
  const HomogenizedThermalConductivity & pps =
      dynamic_cast<const HomogenizedThermalConductivity &>(y);

  _integral_value += pps._integral_value;
  _volume += pps._volume;
}

Real
HomogenizedThermalConductivity::computeQpIntegral()
{
  Real value = 1.0;

  switch (_component)
  {
    case 0:
      value += _grad_temp_x[_qp](0);
      break;

    case 1:
      value += _grad_temp_y[_qp](1);
      break;

    case 2:
      value += _grad_temp_z[_qp](2);
      break;

    default:
      mooseError("Internal error.");
  }

  return _scale * _diffusion_coefficient[_qp] * value;
}
