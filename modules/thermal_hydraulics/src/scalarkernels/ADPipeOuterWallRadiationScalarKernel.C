//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPipeOuterWallRadiationScalarKernel.h"

#include "Function.h"
#include "ThermalSolidProperties.h"
#include "PhysicalConstants.h"

registerMooseObject("ThermalHydraulicsApp", ADPipeOuterWallRadiationScalarKernel);

InputParameters
ADPipeOuterWallRadiationScalarKernel::validParams()
{
  InputParameters params = ADScalarKernel::validParams();
  params.addClassDescription(
      "Adds a radiative heat transfer term, exchanged with an ambient/surrounding "
      "environment, to the outer-surface radial node of a pipe wall.");
  params.addParam<bool>(
      "is_implicit",
      false,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  params.addRequiredParam<UserObjectName>("solid_properties",
                                          "The name of the user object for solid properties");
  params.addRequiredParam<FunctionName>("area", "Cross-sectional area of this wall layer [m^2]");
  params.addRequiredParam<FunctionName>(
      "perimeter", "Perimeter of this wall node exposed to the radiative environment [m]");
  params.addRequiredParam<FunctionName>("T_ambient", "Ambient temperature function [K]");
  params.addRequiredParam<FunctionName>("emissivity", "Emissivity function");
  params.addParam<FunctionName>("view_factor", 1.0, "View factor function");
  params.addParam<FunctionName>(
      "scale", 1.0, "Function by which to scale the radiative heat transfer term");
  params.addParam<Real>("stefan_boltzmann_constant",
                        PhysicalConstants::stefan_boltzmann_constant,
                        "Stefan-Boltzmann constant");

  return params;
}

ADPipeOuterWallRadiationScalarKernel::ADPipeOuterWallRadiationScalarKernel(
    const InputParameters & parameters)
  : Base(parameters),
    _sp(this->template getUserObject<ThermalSolidProperties>("solid_properties")),
    _is_implicit(this->template getParam<bool>("is_implicit")),
    _area(getFunction("area")),
    _perimeter(getFunction("perimeter")),
    _T_ambient(getFunction("T_ambient")),
    _emissivity(getFunction("emissivity")),
    _view_factor(getFunction("view_factor")),
    _scale(getFunction("scale")),
    _sigma(this->template getParam<Real>("stefan_boltzmann_constant"))
{
}

ADReal
ADPipeOuterWallRadiationScalarKernel::computeQpResidual()
{
  const int i = 0;

  const auto A = _area.value(_t, Point());
  const auto cp = _sp.cp_from_T(Base::_u[i]);
  const auto rho = _sp.rho_from_T(Base::_u[i]);

  const auto scale = _scale.value(_t, Point());
  const auto emissivity = _emissivity.value(_t, Point());
  const auto view_factor = _view_factor.value(_t, Point());
  const auto T4 = pow(Base::_u[i], 4);
  const auto T4inf = pow(_T_ambient.value(_t, Point()), 4);

  // Radiative heat transfer with the ambient/surrounding environment
  const auto q_radiation =
      _sigma * scale * emissivity * view_factor * _perimeter.value(_t, Point()) * (T4inf - T4);

  return -q_radiation / A / rho / cp;
}

Real
ADPipeOuterWallRadiationScalarKernel::computeQpJacobian()
{
  mooseError("computeQpJacobian() should not be called in AD mode");
  return 0;
}
