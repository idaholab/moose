//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPipeWallTemperatureScalarKernelBase.h"

#include "Function.h"
#include "ScalarCoupleable.h"
#include "ThermalSolidProperties.h"

InputParameters
ADPipeWallTemperatureScalarKernelBase::validParams()
{
  InputParameters params = ADScalarKernel::validParams();
  params.addClassDescription(
      "Base class for solving the temperature of a radial node of a pipe wall, with axial "
      "conduction to the upstream/downstream nodes of the same radial layer and radial "
      "conduction to the node on the other side of the wall.");
  params.addCoupledVar("upstream_wall_temperature",
                       {},
                       "Wall temperature of the upstream node, same radial layer. Takes a "
                       "scalar variable name");
  params.addCoupledVar("downstream_wall_temperature",
                       {},
                       "Wall temperature of the downstream node, same radial layer. Takes "
                       "a scalar variable name");
  params.addParam<bool>(
      "is_implicit",
      false,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  params.addRequiredParam<UserObjectName>("solid_properties",
                                          "The name of the user object for solid properties");
  params.addRequiredParam<FunctionName>("area", "Cross-sectional area of this wall layer [m^2]");
  params.addRequiredParam<FunctionName>(
      "interface_perimeter",
      "Perimeter of the interface between the inner and outer wall nodes [m]");
  params.addRequiredParam<FunctionName>(
      "wall_thickness",
      "Radial conduction-path distance between the inner and outer wall nodes [m]");
  params.addRequiredParam<FunctionName>("length", "Axial length of this node's control volume [m]");
  params.addRequiredParam<FunctionName>("upstream_spacing",
                                        "Axial distance from this node to the upstream node [m]");
  params.addRequiredParam<FunctionName>("downstream_spacing",
                                        "Axial distance from this node to the downstream node [m]");
  params.addRequiredParam<FunctionName>(
      "upstream_area", "Cross-sectional area of the upstream node's wall layer [m^2]");
  params.addRequiredParam<FunctionName>(
      "downstream_area", "Cross-sectional area of the downstream node's wall layer [m^2]");

  return params;
}

ADPipeWallTemperatureScalarKernelBase::ADPipeWallTemperatureScalarKernelBase(
    const InputParameters & parameters)
  : Base(parameters),
    _sp(getUserObject<ThermalSolidProperties>("solid_properties")),
    _Tup(ScalarCoupleable::adCoupledScalarValue("upstream_wall_temperature")),
    _Tdown(ScalarCoupleable::adCoupledScalarValue("downstream_wall_temperature")),
    _is_implicit(getParam<bool>("is_implicit")),
    _area(getFunction("area")),
    _interface_perimeter(getFunction("interface_perimeter")),
    _wall_thickness(getFunction("wall_thickness")),
    _length(getFunction("length")),
    _upstream_spacing(getFunction("upstream_spacing")),
    _downstream_spacing(getFunction("downstream_spacing")),
    _upstream_area(getFunction("upstream_area")),
    _downstream_area(getFunction("downstream_area"))
{
}

ADReal
ADPipeWallTemperatureScalarKernelBase::computeQpResidual()
{
  const int i = 0;

  const auto A = _area.value(_t, Point());
  const auto k = _sp.k_from_T(Base::_u[i]);
  const auto cp = _sp.cp_from_T(Base::_u[i]);
  const auto rho = _sp.rho_from_T(Base::_u[i]);

  // Neighbor wall-layer cross-sectional areas and conductivities, used below to build a
  // harmonic-mean face conductance for axial conduction between control volumes whose
  // cross-section may differ (two half-length resistances in series)
  const auto A_up = _upstream_area.value(_t, Point());
  const auto k_up = _sp.k_from_T(_Tup[i]);
  const auto A_down = _downstream_area.value(_t, Point());
  const auto k_down = _sp.k_from_T(_Tdown[i]);

  const auto G_up = 1.0 / (_upstream_spacing.value(_t, Point()) / (2.0 * k * A) +
                           _upstream_spacing.value(_t, Point()) / (2.0 * k_up * A_up));
  const auto G_down = 1.0 / (_downstream_spacing.value(_t, Point()) / (2.0 * k * A) +
                             _downstream_spacing.value(_t, Point()) / (2.0 * k_down * A_down));

  // Radial conduction from the node on the other side of this layer
  const auto q_radial = k * _interface_perimeter.value(_t, Point()) *
                        (otherNodeTemperature() - Base::_u[i]) / _wall_thickness.value(_t, Point());
  // Axial conduction from the upstream/downstream nodes of this layer
  const auto q_axial = (G_up * (_Tup[i] - Base::_u[i]) + G_down * (_Tdown[i] - Base::_u[i])) /
                       _length.value(_t, Point());
  // Heat exchanged with the fluid or ambient environment on this node's non-radial side
  const auto q_convective = convectiveLinearHeatRate();

  return -(q_radial + q_axial + q_convective) / (rho * cp * A);
}

Real
ADPipeWallTemperatureScalarKernelBase::computeQpJacobian()
{
  mooseError("computeQpJacobian() should not be called in AD mode");
  return 0;
}
