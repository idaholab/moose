//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AriaLaserWeld304LStainlessSteel.h"

registerMooseObject("NavierStokesTestApp", AriaLaserWeld304LStainlessSteel);

InputParameters
AriaLaserWeld304LStainlessSteel::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<Real>("c_mu0", 0.15616, "mu0 coefficient");
  params.addParam<Real>("c_mu1", -3.3696e-5, "mu1 coefficient");
  params.addParam<Real>("c_mu2", 1.0191e-8, "mu2 coefficient");
  params.addParam<Real>("c_mu3", -1.0413e-12, "mu3 coefficient");
  params.addParam<Real>("Tmax", 4000, "The maximum temperature");
  params.addParam<Real>("Tl", 1623, "The liquidus temperature");
  params.addParam<Real>(
      "T90", 1528, "The T90 temperature (I don't know what this means physically)");
  params.addParam<Real>("beta", 1e11, "beta coefficient");
  params.addParam<Real>("c_k0", 10.7143, "k0 coefficient");
  params.addParam<Real>("c_k1", 14.2857e-3, "k1 coefficient");
  params.addParam<Real>("c_cp0", 425.75, "cp0 coefficient");
  params.addParam<Real>("c_cp1", 170.833e-3, "cp1 coefficient");
  params.addParam<Real>("c_rho0", 7.9e3, "The constant density");
  params.addRequiredCoupledVar("temperature", "The temperature in K");
  params.addParam<MaterialPropertyName>(
      "mu_name", "mu", "The name of the viscosity material property");
  params.addParam<MaterialPropertyName>("k_name", "k", "The name of the thermal conductivity");
  params.addParam<MaterialPropertyName>("cp_name", "cp", "The name of the thermal conductivity");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the thermal conductivity");
  return params;
}

AriaLaserWeld304LStainlessSteel::AriaLaserWeld304LStainlessSteel(const InputParameters & parameters)
  : Material(parameters),
    _c_mu0(getParam<Real>("c_mu0")),
    _c_mu1(getParam<Real>("c_mu1")),
    _c_mu2(getParam<Real>("c_mu2")),
    _c_mu3(getParam<Real>("c_mu3")),
    _Tmax(getParam<Real>("Tmax")),
    _Tl(getParam<Real>("Tl")),
    _T90(getParam<Real>("T90")),
    _beta(getParam<Real>("beta")),
    _c_k0(getParam<Real>("c_k0")),
    _c_k1(getParam<Real>("c_k1")),
    _c_cp0(getParam<Real>("c_cp0")),
    _c_cp1(getParam<Real>("c_cp1")),
    _c_rho0(getParam<Real>("c_rho0")),
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
AriaLaserWeld304LStainlessSteel::computeQpProperties()
{
  if (_temperature[_qp] < _Tl)
    _mu[_qp] = (_c_mu0 + _c_mu1 * _Tl + _c_mu2 * _Tl * _Tl + _c_mu3 * _Tl * _Tl * _Tl) *
               (_beta + (1 - _beta) * (_temperature[_qp] - _T90) / (_Tl - _T90));
  else
  {
    ADReal That;
    That = _temperature[_qp] > _Tmax ? _Tmax : _temperature[_qp];
    _mu[_qp] = (_c_mu0 + _c_mu1 * That + _c_mu2 * That * That + _c_mu3 * That * That * That);
  }
  _k[_qp] = _c_k0 + _c_k1 * _temperature[_qp];
  _grad_k[_qp] = _c_k1 * _grad_temperature[_qp];
  _cp[_qp] = _c_cp0 + _c_cp1 * _temperature[_qp];
  _rho[_qp] = _c_rho0;
}
