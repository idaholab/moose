//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LaserWeld316LStainlessSteelBoundary.h"
#include "Assembly.h"

registerMooseObject("NavierStokesTestApp", LaserWeld316LStainlessSteelBoundary);

InputParameters
LaserWeld316LStainlessSteelBoundary::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<Real>("P0", 1e5, "The ambient pressure for the recoil pressure");
  params.addParam<Real>("L_v", 7.45E6, "Latent heat from evaporation for the recoil pressure");
  params.addParam<Real>("M", 56E-3, "Molar mass for the recoil pressure");
  params.addParam<Real>("T_v", 3080, "The vaporization temperature for recoil pressure");
  params.addParam<Real>("R", 8.314, "The gas constant for recoil pressure");
  params.addParam<Real>("c_gamma0", 1.593, "Constant term in the surface tension");
  params.addParam<Real>("c_gamma1", 1.9e-4, "Linear term multiplier in the surface tension");
  params.addParam<Real>("Tl", 1708, "The liquidus temperature");
  params.addRequiredCoupledVar("temperature", "The temperature in K");
  params.addParam<MaterialPropertyName>("rc_pressure_name", "rc_pressure", "The recoil pressure");
  params.addParam<MaterialPropertyName>(
      "surface_tension_name", "surface_tension", "The surface tension");
  params.addParam<MaterialPropertyName>(
      "grad_surface_tension_name", "grad_surface_tension", "The gradient of the surface tension");
  return params;
}

LaserWeld316LStainlessSteelBoundary::LaserWeld316LStainlessSteelBoundary(
    const InputParameters & parameters)
  : Material(parameters),
    _P0(getParam<Real>("P0")),
    _L_v(getParam<Real>("L_v")),
    _M(getParam<Real>("M")),
    _T_v(getParam<Real>("T_v")),
    _R(getParam<Real>("R")),
    _c_gamma0(getParam<Real>("c_gamma0")),
    _c_gamma1(getParam<Real>("c_gamma1")),
    _Tl(getParam<Real>("Tl")),
    _rc_pressure(declareADProperty<Real>(getParam<MaterialPropertyName>("rc_pressure_name"))),
    _surface_tension(
        declareADProperty<Real>(getParam<MaterialPropertyName>("surface_tension_name"))),
    _grad_surface_tension(declareADProperty<RealVectorValue>(
        getParam<MaterialPropertyName>("grad_surface_tension_name"))),
    _surface_term_curvature(declareADProperty<RealVectorValue>("surface_term_curvature")),
    _surface_term_gradient1(declareADProperty<RealVectorValue>("surface_term_gradient1")),
    _surface_term_gradient2(declareADProperty<RealVectorValue>("surface_term_gradient2")),
    _ad_normals(_assembly.adNormals()),
    _ad_curvatures(_assembly.adCurvatures()),
    _temperature(adCoupledValue("temperature")),
    _grad_temperature(adCoupledGradient("temperature"))
{
}

void
LaserWeld316LStainlessSteelBoundary::computeQpProperties()
{
  // Below the vaporization temperature we don't have recoil pressure
  if (_temperature[_qp] < _T_v)
    _rc_pressure[_qp] = 0;
  else
    _rc_pressure[_qp] =
        0.54 * _P0 *
        std::exp(_L_v * _M / _R * (_temperature[_qp] - _T_v) / (_temperature[_qp] * _T_v));

  // The surface tension treatment methodology is from:
  // Noble, David R et al, Use of Aria to simulate laser weld pool dynamics for neutron generator production,
  // 2007, Sandia National Laboratories (SNL), Albuquerque, NM, and Livermore, CA
  //
  // with the surface tension from:
  //
  // Pichler, Peter, et al. "Surface tension and thermal conductivity of NIST SRM 1155a
  // (AISI 316L stainless steel)."
  _surface_tension[_qp] = _c_gamma0 + _c_gamma1 * (_temperature[_qp] - _Tl);
  _grad_surface_tension[_qp] = _c_gamma1 * _grad_temperature[_qp];

  // These terms are needed for the treatment in:
  // Cairncross RA, Schunk PR, Baer TA, Rao RR, Sackinger PA. A finite element method for free surface
  // flows of incompressible fluids in three dimensions. Part I. Boundary fitted mesh motion.
  // International journal for numerical methods in fluids. 2000 Jun 15;33(3):375-403.
  _surface_term_curvature[_qp] =
      -2. * _ad_curvatures[_qp] * _surface_tension[_qp] * _ad_normals[_qp];
  _surface_term_gradient1[_qp] = -_grad_surface_tension[_qp];
  _surface_term_gradient2[_qp] = _ad_normals[_qp] * (_ad_normals[_qp] * _grad_surface_tension[_qp]);
}
