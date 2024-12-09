//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AriaLaserWeld304LStainlessSteelBoundary.h"
#include "Assembly.h"

registerMooseObject("NavierStokesTestApp", AriaLaserWeld304LStainlessSteelBoundary);

InputParameters
AriaLaserWeld304LStainlessSteelBoundary::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<Real>("c_mu0", 0.15616, "mu0 coefficient");
  params.addParam<Real>("ap0", 0, "");
  params.addParam<Real>("ap1", 1.851502e1, "");
  params.addParam<Real>("ap2", -1.96945e-1, "");
  params.addParam<Real>("ap3", 1.594124e-3, "");
  params.addParam<Real>("bp0", 0, "");
  params.addParam<Real>("bp1", -5.809553e1, "");
  params.addParam<Real>("bp2", 4.610515e-1, "");
  params.addParam<Real>("bp3", 2.332819e-4, "");
  params.addParam<Real>("Tb", 3000, "The boiling temperature");
  params.addParam<Real>("Tbound1", 0, "The first temperature bound");
  params.addParam<Real>("Tbound2", 170, "The second temperature bound");
  params.addRequiredCoupledVar("temperature", "The temperature in K");
  params.addParam<MaterialPropertyName>("rc_pressure_name", "rc_pressure", "The recoil pressure");
  params.addParam<Real>(
      "alpha", -4.3e-4, "The derivative of the surface tension with respect to temperature");
  params.addParam<Real>("sigma0", 1.943, "The surface tension at T0");
  params.addParam<Real>("T0", 1809, "The reference temperature for the surface tension");
  params.addParam<MaterialPropertyName>(
      "surface_tension_name", "surface_tension", "The surface tension");
  params.addParam<MaterialPropertyName>(
      "grad_surface_tension_name", "grad_surface_tension", "The gradient of the surface tension");
  return params;
}

AriaLaserWeld304LStainlessSteelBoundary::AriaLaserWeld304LStainlessSteelBoundary(
    const InputParameters & parameters)
  : Material(parameters),
    _ap0(getParam<Real>("ap0")),
    _ap1(getParam<Real>("ap1")),
    _ap2(getParam<Real>("ap2")),
    _ap3(getParam<Real>("ap3")),
    _bp0(getParam<Real>("bp0")),
    _bp1(getParam<Real>("bp1")),
    _bp2(getParam<Real>("bp2")),
    _bp3(getParam<Real>("bp3")),
    _Tb(getParam<Real>("Tb")),
    _Tbound1(getParam<Real>("Tbound1")),
    _Tbound2(getParam<Real>("Tbound2")),
    _temperature(adCoupledValue("temperature")),
    _grad_temperature(adCoupledGradient("temperature")),
    _rc_pressure(declareADProperty<Real>(getParam<MaterialPropertyName>("rc_pressure_name"))),
    _alpha(getParam<Real>("alpha")),
    _sigma0(getParam<Real>("sigma0")),
    _T0(getParam<Real>("T0")),
    _surface_tension(
        declareADProperty<Real>(getParam<MaterialPropertyName>("surface_tension_name"))),
    _grad_surface_tension(declareADProperty<RealVectorValue>(
        getParam<MaterialPropertyName>("grad_surface_tension_name"))),
    _ad_normals(_assembly.adNormals()),
    _ad_curvatures(_assembly.adCurvatures()),
    _surface_term_curvature(declareADProperty<RealVectorValue>("surface_term_curvature")),
    _surface_term_gradient1(declareADProperty<RealVectorValue>("surface_term_gradient1")),
    _surface_term_gradient2(declareADProperty<RealVectorValue>("surface_term_gradient2"))
{
}

void
AriaLaserWeld304LStainlessSteelBoundary::computeQpProperties()
{
  const auto theta = _temperature[_qp] - _Tb;
  if (theta < _Tbound1)
    _rc_pressure[_qp] = 0;
  else if (theta < _Tbound2)
    _rc_pressure[_qp] = _ap0 + _ap1 * theta + _ap2 * theta * theta + _ap3 * theta * theta * theta;
  else
    _rc_pressure[_qp] = _bp0 + _bp1 * theta + _bp2 * theta * theta + _bp3 * theta * theta * theta;

  _surface_tension[_qp] = _sigma0 + _alpha * (_temperature[_qp] - _T0);
  _grad_surface_tension[_qp] = _alpha * _grad_temperature[_qp];
  _surface_term_curvature[_qp] =
      -2. * _ad_curvatures[_qp] * _surface_tension[_qp] * _ad_normals[_qp];
  _surface_term_gradient1[_qp] = -_grad_surface_tension[_qp];
  _surface_term_gradient2[_qp] = _ad_normals[_qp] * (_ad_normals[_qp] * _grad_surface_tension[_qp]);
}
