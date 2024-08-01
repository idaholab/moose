//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MixedLaserWeld316LStainlessSteel.h"

registerMooseObject("NavierStokesTestApp", MixedLaserWeld316LStainlessSteel);

InputParameters
MixedLaserWeld316LStainlessSteel::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("temperature", "The temperature in K");

  params.addParam<Real>("c_mu0", 2385.2, "mu0 coefficient in exp(m0/T-mu1)");
  params.addParam<Real>("c_mu1", 0.5958, "mu1 coefficient in exp(m0/T-mu1)");

  params.addParam<Real>("Tmax", 4000, "The maximum temperature for limiting viscosity");
  params.addParam<Real>("Tl", 1708, "The liquidus temperature");

  params.addParam<Real>("c_k0_s", 9.248, "k0 coefficient for the solid in k0+k1*T");
  params.addParam<Real>("c_k1_s", 1.571E-2, "k1 coefficient for the solid in k0+k1*T");
  params.addParam<Real>("c_k0_l", 12.41, "k0 coefficient for the liquid in k0+k1*T");
  params.addParam<Real>("c_k1_l", 3.279E-3, "k1 coefficien for the liquid in k0+k1*Tt");

  params.addParam<Real>("c_cp0_s", 458.98, "cp0 coefficient for the solid in cp0+cp1*T");
  params.addParam<Real>("c_cp1_s", 0.1328, "cp1 coefficient for the solid in cp0+cp1*T");
  params.addParam<Real>("c_cp0_s", 794.96, "cp0 coefficient for the liquid in cp0");

  params.addParam<Real>(
      "c_rho0_s", 8084.2, "The rho0 density for the solid in rho0+rho1*T+rho2*T*T");
  params.addParam<Real>(
      "c_rho1_s", -0.42086, "The rho1 density for the solid in rho0+rho1*T+rho2*T*T");
  params.addParam<Real>(
      "c_rho2_s", -3.8942E-5, "The rho2 density for the solid in rho0+rho1*T+rho2*T*T");
  params.addParam<Real>(
      "c_rho0_l", 7432.7, "The rho0 density for the liquid in rho0+rho1*T+rho2*T*T");
  params.addParam<Real>(
      "c_rho1_l", 0.039338, "The rho1 density for the liquid in rho0+rho1*T+rho2*T*T");
  params.addParam<Real>(
      "c_rho2_l", -1.8007E-4, "The rho2 density for the liquid in rho0+rho1*T+rho2*T*T");

  params.addParam<MaterialPropertyName>(
      "mu_name", "mu", "The name of the viscosity material property");
  params.addParam<MaterialPropertyName>("k_name", "k", "The name of the thermal conductivity");
  params.addParam<MaterialPropertyName>("cp_name", "cp", "The name of the specific heat capacity");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
  return params;
}

MixedLaserWeld316LStainlessSteel::MixedLaserWeld316LStainlessSteel(
    const InputParameters & parameters)
  : Material(parameters),
    _c_mu0(getParam<Real>("c_mu0")),
    _c_mu1(getParam<Real>("c_mu1")),
    _c_k0_s(getParam<Real>("c_k0_s")),
    _c_k1_s(getParam<Real>("c_k1_s")),
    _c_k0_l(getParam<Real>("c_k0_l")),
    _c_k1_l(getParam<Real>("c_k1_l")),
    _c_cp0_s(getParam<Real>("c_cp0_s")),
    _c_cp1_s(getParam<Real>("c_cp1_s")),
    _c_cp0_l(getParam<Real>("c_cp0_l")),
    _c_rho0_s(getParam<Real>("c_rho0_s")),
    _c_rho1_s(getParam<Real>("c_rho0_s")),
    _c_rho2_s(getParam<Real>("c_rho0_s")),
    _c_rho0_l(getParam<Real>("c_rho0_l")),
    _c_rho1_l(getParam<Real>("c_rho0_l")),
    _c_rho2_l(getParam<Real>("c_rho0_l")),
    _Tmax(getParam<Real>("Tmax")),
    _Tl(getParam<Real>("Tl")),
    _temperature(adCoupledValue("temperature")),
    _grad_temperature(adCoupledGradient("temperature")),
    _mu(declareADProperty<Real>(getParam<MaterialPropertyName>("mu_name"))),
    _k(declareADProperty<Real>(getParam<MaterialPropertyName>("k_name"))),
    _cp(declareADProperty<Real>(getParam<MaterialPropertyName>("cp_name"))),
    _rho(declareADProperty<Real>(getParam<MaterialPropertyName>("rho_name"))),
    _grad_k(declareADProperty<RealVectorValue>("grad_" + getParam<MaterialPropertyName>("k_name")))
{
}

void
MixedLaserWeld316LStainlessSteel::computeQpProperties()
{
  if (_temperature[_qp] < _Tl)
  {
    _k[_qp] = _c_k0_s + _c_k1_s * _temperature[_qp];
    _grad_k[_qp] = _c_k1_s * _grad_temperature[_qp];
    _cp[_qp] = _c_cp0_s + _c_cp1_s * _temperature[_qp];
    _rho[_qp] = _c_rho0_s + _c_rho1_s * _temperature[_qp] +
                _c_rho2_s * _temperature[_qp] * _temperature[_qp];
  }
  else
  {
    _k[_qp] = _c_k0_l + _c_k1_l * _temperature[_qp];
    _grad_k[_qp] = _c_k1_l * _grad_temperature[_qp];
    _cp[_qp] = _c_cp0_l;
    _rho[_qp] = _c_rho0_l + _c_rho1_l * _temperature[_qp] +
                _c_rho2_l * _temperature[_qp] * _temperature[_qp];
  }
  ADReal That;
  That = _temperature[_qp] > _Tmax ? _Tmax : _temperature[_qp];
  _mu[_qp] = 1e-3 * std::exp(_c_mu0 / That - _c_mu1);
}
