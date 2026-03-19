//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HenryGasConstant.h"
#include "MathUtils.h"

registerMooseObject("ChemicalReactionsApp", HenryGasConstant);

InputParameters
HenryGasConstant::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<Real>("radius", "Van der Waals radius [m]");
  MooseEnum salt_list("FLIBE FLINAK CUSTOM");
  params.addRequiredParam<MooseEnum>("salt", salt_list, "Salt");
  params.addParam<Real>("alpha", "alpha fit parameter in Henry model, if defining custom fluid");
  params.addParam<Real>("beta", "beta fit parameter in Henry model, if defining custom fluid");
  params.addParam<Real>("gamma_0",
                        "gamma_0 fit parameter in Henry model, if defining custom fluidl");
  params.addParam<Real>("dgamma_dT",
                        "gamma derivative fit parameter in Henry model, if defining custom fluid");
  params.addParam<Real>(
      "KH0", "Reference Henry parameter in Henry model, if defining custom fluid [mol/m3/Pa]");
  params.addClassDescription("Calculates Henry gas constant of a noble gas `[mol/m3/Pa]`");
  return params;
}

HenryGasConstant::HenryGasConstant(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _radius(getParam<Real>("radius")),
    _salt_list(getParam<MooseEnum>("salt").getEnum<Saltlist>()),
    _alpha(isParamSetByUser("alpha") ? getParam<Real>("alpha") : 0.0),
    _beta(isParamSetByUser("beta") ? getParam<Real>("beta") : 0.0),
    _gamma_0(isParamSetByUser("gamma_0") ? getParam<Real>("gamma_0") : 0.0),
    _dgamma_dT(isParamSetByUser("dgamma_dT") ? getParam<Real>("dgamma_dT") : 0.0),
    _KH0(isParamSetByUser("KH0") ? getParam<Real>("KH0") : 0.0)
{
  switch (_salt_list)
  {
    case Saltlist::FLIBE:
      _alpha = _alpha_FLiBe;
      _beta = _beta_FLiBe;
      _KH0 = _KH0_FLiBe;
      _gamma_0 = _gamma_0_FLiBe;
      _dgamma_dT = _dgamma_dT_FLiBe;
      break;

    case Saltlist::FLINAK:
      _alpha = _alpha_FLiNaK;
      _beta = _beta_FLiNaK;
      _KH0 = _KH0_FLiNaK;
      _gamma_0 = _gamma_0_FLiNaK;
      _dgamma_dT = _dgamma_dT_FLiNaK;
      break;

    case Saltlist::CUSTOM:
      // Ensure that all parameters were set by user
      if (!isParamSetByUser("alpha"))
        mooseError("Must include alpha parameter when using custom salt Henry gas model");
      if (!isParamSetByUser("beta"))
        mooseError("Must include beta parameter when using custom salt Henry gas model");
      if (!isParamSetByUser("gamma_0"))
        mooseError("Must include gamma_0 parameter when using custom salt Henry gas model");
      if (!isParamSetByUser("dgamma_dT"))
        mooseError("Must include dgamma_dT parameter when using custom salt Henry gas model");
      if (!isParamSetByUser("KH0"))
        mooseError("Must include KH0 parameter when using custom salt Henry gas model");
      break;
  }
}

Real
HenryGasConstant::henry(Real temperature) const
{

  const Real m_to_ang = 1e10;

  // Equations result in units of mol/cm^3/atm, so convert to mol/m^3/atm
  Real value =
      _alpha * 4. * libMesh::pi * Utility::pow<2>(_radius * m_to_ang) *
          (_gamma_0 + _dgamma_dT * (temperature - 273.15)) +
      _beta * 4. / 3. * libMesh::pi * std::pow(_radius * m_to_ang, 3) * _Rgas * temperature;
  value = exp(value / (_Rgas * temperature)) * _KH0;
  return value;
}
