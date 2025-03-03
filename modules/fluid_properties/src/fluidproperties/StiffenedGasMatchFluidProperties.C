//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StiffenedGasMatchFluidProperties.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", StiffenedGasMatchFluidProperties);

InputParameters
StiffenedGasMatchFluidProperties::validParams()
{
  InputParameters params = StiffenedGasFluidPropertiesBase::validParams();

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "SinglePhaseFluidProperties object to match");
  params.addRequiredParam<Real>("p", "Pressure at which to match properties [Pa]");
  params.addRequiredParam<Real>("T", "Temperature at which to match properties [K]");
  params.addParam<bool>("require_critical_properties",
                        false,
                        "If true, require that the reference SinglePhaseFluidProperties has "
                        "implemented the critical property methods.");

  params.addClassDescription(
      "Stiffened gas fluid properties that fit another fluid at a (p,T) state.");

  return params;
}

StiffenedGasMatchFluidProperties::StiffenedGasMatchFluidProperties(
    const InputParameters & parameters)
  : StiffenedGasFluidPropertiesBase(parameters),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties")),
    _p(getParam<Real>("p")),
    _T(getParam<Real>("T"))
{
}

void
StiffenedGasMatchFluidProperties::initialSetupInner()
{
  _cp = _fp.cp_from_p_T(_p, _T);
  _cv = _fp.cv_from_p_T(_p, _T);
  _gamma = _cp / _cv;
  _molar_mass = _fp.molarMass();

  // from h(T) = gamma * cv * T + q
  _q = _fp.h_from_p_T(_p, _T) - _gamma * _cv * _T;

  // from rho(p,T) = (p + p_inf) / [(gamma - 1) * cv * T]
  _p_inf = _fp.rho_from_p_T(_p, _T) * (_gamma - 1.0) * _cv * _T - _p;

  // from s(p,T) = cv * ln[T^gamma/(p + p_inf)^(gamma - 1)] + q_prime
  _q_prime = _fp.s_from_p_T(_p, _T) -
             _cv * std::log(std::pow(_T, _gamma) / std::pow(_p + _p_inf, _gamma - 1.0));

  _mu = _fp.mu_from_p_T(_p, _T);
  _k = _fp.k_from_p_T(_p, _T);

  if (getParam<bool>("require_critical_properties"))
  {
    _T_c = _fp.criticalTemperature();
    _rho_c = _fp.criticalDensity();
    _e_c = _fp.criticalInternalEnergy();
  }
  else
  {
    _T_c = getNaN();
    _rho_c = getNaN();
    _e_c = getNaN();
  }
}
