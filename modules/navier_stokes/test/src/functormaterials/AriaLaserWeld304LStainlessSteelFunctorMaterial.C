//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AriaLaserWeld304LStainlessSteelFunctorMaterial.h"
#include "NS.h"

registerMooseObject("NavierStokesTestApp", AriaLaserWeld304LStainlessSteelFunctorMaterial);

InputParameters
AriaLaserWeld304LStainlessSteelFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
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
  params.addRequiredParam<MooseFunctorName>(NS::temperature, "The temperature in K");
  params.addParam<MooseFunctorName>(NS::mu, NS::mu, "The name of the viscosity material property");
  params.addParam<MooseFunctorName>(NS::k, NS::k, "The name of the thermal conductivity");
  params.addParam<MooseFunctorName>(NS::cp, NS::cp, "The name of the specific heat capacity");
  params.addParam<MooseFunctorName>(NS::density, NS::density, "The name of the density");
  params.addParam<MooseFunctorName>("rc_pressure", "rc_pressure", "The recoil pressure");
  return params;
}

AriaLaserWeld304LStainlessSteelFunctorMaterial::AriaLaserWeld304LStainlessSteelFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
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
    _temperature(getFunctor<ADReal>(NS::temperature))
{
  addFunctorProperty<ADReal>(
      NS::mu,
      [this](const auto & r, const auto & t)
      {
        const auto T = _temperature(r, t);
        if (MetaPhysicL::raw_value(T) < _Tl)
          return (_c_mu0 + _c_mu1 * _Tl + _c_mu2 * _Tl * _Tl + _c_mu3 * _Tl * _Tl * _Tl) *
                 (_beta + (1 - _beta) * (T - _T90) / (_Tl - _T90));
        else
        {
          const ADReal That = T > _Tmax ? _Tmax : T;
          return (_c_mu0 + _c_mu1 * That + _c_mu2 * That * That + _c_mu3 * That * That * That);
        }
      });
  addFunctorProperty<ADReal>(
      NS::k, [this](const auto & r, const auto & t) { return _c_k0 + _c_k1 * _temperature(r, t); });
  addFunctorProperty<ADReal>(NS::cp,
                             [this](const auto & r, const auto & t)
                             { return _c_cp0 + _c_cp1 * _temperature(r, t); });
  addFunctorProperty<ADReal>(NS::density, [this](const auto &, const auto &) { return _c_rho0; });
  addFunctorProperty<ADReal>(
      "rc_pressure",
      [this](const auto & r, const auto & t)
      {
        const auto theta = _temperature(r, t) - _Tb;
        if (theta < _Tbound1)
          return ADReal(0);
        else if (theta < _Tbound2)
          return _ap0 + _ap1 * theta + _ap2 * theta * theta + _ap3 * theta * theta * theta;
        else
          return _bp0 + _bp1 * theta + _bp2 * theta * theta + _bp3 * theta * theta * theta;
      });
}
