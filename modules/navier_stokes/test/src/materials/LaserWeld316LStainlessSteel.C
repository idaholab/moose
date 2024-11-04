//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LaserWeld316LStainlessSteel.h"

registerMooseObject("NavierStokesTestApp", LaserWeld316LStainlessSteel);

InputParameters
LaserWeld316LStainlessSteel::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("temperature", "The temperature in K");

  params.addParam<Real>("c_mu0", 2385.2, "mu0 coefficient in exp(m0/T-mu1)");
  params.addParam<Real>("c_mu1", 0.5958, "mu1 coefficient in exp(m0/T-mu1)");

  params.addParam<Real>("Tmax", 4000, "The maximum temperature for limiting viscosity");
  params.addParam<Real>("Tl", 1708, "The liquidus temperature");
  params.addParam<Real>("Ts", 1675, "The solidus temperature");

  params.addParam<Real>("c_k0_s", 6.38, "k0 coefficient for the solid in k0+k1*T");
  params.addParam<Real>("c_k1_s", 1.9E-2, "k1 coefficient for the solid in k0+k1*T");
  params.addParam<Real>("c_k2_s", -2.45E-6, "k1 coefficient for the solid in k0+k1*T");
  params.addParam<Real>("c_k0_l", 2.27, "k0 coefficient for the liquid in k0+k1*T");
  params.addParam<Real>("c_k1_l", 1.76E-2, "k1 coefficien for the liquid in k0+k1*Tt");
  params.addParam<Real>("c_k2_l", -1.39E-6, "k1 coefficient for the solid in k0+k1*T");

  params.addParam<Real>("c_cp0_s", 459.29, "cp0 coefficient for the solid in cp0+cp1*T");
  params.addParam<Real>("c_cp1_s", 0.1329, "cp1 coefficient for the solid in cp0+cp1*T");
  params.addParam<Real>("c_cp0_l", 795.49, "cp0 coefficient for the liquid in cp0");

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

  params.addParam<bool>("use_constant_density", true, "If a constant density should be used (would discard buoyancy effects)");

  return params;
}

LaserWeld316LStainlessSteel::LaserWeld316LStainlessSteel(
    const InputParameters & parameters)
  : Material(parameters),
    _c_mu0(getParam<Real>("c_mu0")),
    _c_mu1(getParam<Real>("c_mu1")),
    _c_k0_s(getParam<Real>("c_k0_s")),
    _c_k1_s(getParam<Real>("c_k1_s")),
    _c_k2_s(getParam<Real>("c_k2_s")),
    _c_k0_l(getParam<Real>("c_k0_l")),
    _c_k1_l(getParam<Real>("c_k1_l")),
    _c_k2_l(getParam<Real>("c_k2_l")),
    _c_cp0_s(getParam<Real>("c_cp0_s")),
    _c_cp1_s(getParam<Real>("c_cp1_s")),
    _c_cp0_l(getParam<Real>("c_cp0_l")),
    _c_rho0_s(getParam<Real>("c_rho0_s")),
    _c_rho1_s(getParam<Real>("c_rho1_s")),
    _c_rho2_s(getParam<Real>("c_rho2_s")),
    _c_rho0_l(getParam<Real>("c_rho0_l")),
    _c_rho1_l(getParam<Real>("c_rho1_l")),
    _c_rho2_l(getParam<Real>("c_rho2_l")),
    _Tmax(getParam<Real>("Tmax")),
    _Tl(getParam<Real>("Tl")),
    _Ts(getParam<Real>("Ts")),
    _temperature(adCoupledValue("temperature")),
    _grad_temperature(adCoupledGradient("temperature")),
    _mu(declareADProperty<Real>(getParam<MaterialPropertyName>("mu_name"))),
    _k(declareADProperty<Real>(getParam<MaterialPropertyName>("k_name"))),
    _cp(declareADProperty<Real>(getParam<MaterialPropertyName>("cp_name"))),
    _rho(declareADProperty<Real>(getParam<MaterialPropertyName>("rho_name"))),
    _grad_k(declareADProperty<RealVectorValue>("grad_" + getParam<MaterialPropertyName>("k_name"))),
    _use_constant_density(getParam<bool>("use_constant_density"))
{
}

void
LaserWeld316LStainlessSteel::computeQpProperties()
{
  ADReal That = _temperature[_qp] > _Tmax ? _Tmax : _temperature[_qp];
  // First we check if it is enough to compute the solid properties only
  if (_temperature[_qp] < _Ts)
  {
    // We are using an enormous viscosity for the solid phase to prevent it from moving.
    // This approach was coined in:
    // Noble, David R et al, Use of Aria to simulate laser weld pool dynamics for neutron generator production,
    // 2007, Sandia National Laboratories (SNL), Albuquerque, NM, and Livermore, CA
    _mu[_qp] = 1e11 * 1e-3 * std::exp(_c_mu0 / That - _c_mu1);
    _k[_qp] =
        _c_k0_s + _c_k1_s * _temperature[_qp] + _c_k2_s * _temperature[_qp] * _temperature[_qp];
    _grad_k[_qp] = _c_k1_s * _grad_temperature[_qp];
    _cp[_qp] = _c_cp0_s + _c_cp1_s * _temperature[_qp];
    _rho[_qp] = _c_rho0_s + _c_rho1_s * _temperature[_qp] +
                _c_rho2_s * _temperature[_qp] * _temperature[_qp];
  }
  else
  {
    // This contains an artifically large lower viscosity at this point, to make sure
    // the fluid simulations are stable. This viscosity is still below the one used in:
    // Noble, David R et al, Use of Aria to simulate laser weld pool dynamics for neutron generator production,
    // 2007, Sandia National Laboratories (SNL), Albuquerque, NM, and Livermore, CA
    const auto mu_l = std::max(0.030, 1e-3 * std::exp(_c_mu0 / That - _c_mu1));
    const auto k_l =
        _c_k0_l + _c_k1_l * _temperature[_qp] + _c_k2_l * _temperature[_qp] * _temperature[_qp];
    const auto grad_k_l = _c_k1_l * _grad_temperature[_qp];
    const auto cp_l = _c_cp0_l;
    const auto rho_l = _c_rho0_l + _c_rho1_l * _temperature[_qp] +
                       _c_rho2_l * _temperature[_qp] * _temperature[_qp];

    // If we are between the solidus and liquidus temperatures we will need the
    // solid properties too
    if (_temperature[_qp] < _Tl && _temperature[_qp] > _Ts)
    {
      const auto liquid_fraction = (_temperature[_qp] - _Ts) / (_Tl - _Ts);

      const auto mu_s = 1e11 * 1e-3 * std::exp(_c_mu0 / That - _c_mu1);
      const auto k_s =
          _c_k0_s + _c_k1_s * _temperature[_qp] + _c_k2_s * _temperature[_qp] * _temperature[_qp];
      const auto grad_k_s = _c_k1_s * _grad_temperature[_qp];
      const auto cp_s = _c_cp0_s + _c_cp1_s * _temperature[_qp];
      const auto rho_s = _c_rho0_s + _c_rho1_s * _temperature[_qp] +
                         _c_rho2_s * _temperature[_qp] * _temperature[_qp];

      _mu[_qp] = liquid_fraction * mu_l + (1 - liquid_fraction) * mu_s;
      _k[_qp] = liquid_fraction * k_l + (1 - liquid_fraction) * k_s;
      _grad_k[_qp] = liquid_fraction * grad_k_l + (1 - liquid_fraction) * grad_k_s;
      _cp[_qp] = liquid_fraction * cp_l + (1 - liquid_fraction) * cp_s;
      _rho[_qp] = liquid_fraction * rho_l + (1 - liquid_fraction) * rho_s;
    }
    else
    {
      _mu[_qp] = mu_l;
      _k[_qp] = k_l;
      _grad_k[_qp] = grad_k_l;
      _cp[_qp] = cp_l;
      _rho[_qp] = rho_l;
    }
  }

  // If we use constant density we override it with the density at the liquidus line
  if (_use_constant_density)
    _rho[_qp] = _c_rho0_l + _c_rho1_l * _Tl +
                       _c_rho2_l * _Tl * _Tl;
}
