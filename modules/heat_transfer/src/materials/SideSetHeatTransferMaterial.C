//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetHeatTransferMaterial.h"

#include "Function.h"

// Stefan - Boltzmann Constant
const Real SIGMA = 5.670374419E-8;

registerMooseObject("HeatConductionApp", SideSetHeatTransferMaterial);

InputParameters
SideSetHeatTransferMaterial::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();
  params.addClassDescription("This material constructs the necessary coefficients and properties "
                             "for SideSetHeatTransferKernel.");
  params.addParam<FunctionName>("conductivity", 0.0, "Heat conductivity in W/m/K.");
  params.addParam<FunctionName>("conductivity_temperature_function",
                                "Heat conductivity in W/m/K as a function of temperature.");
  params.addCoupledVar(
      "gap_temperature",
      "Coupled Temperature of gap, used for computing temperature dependent conductivity.");
  params.addParam<FunctionName>("gap_length", 1.0, "Total width of gap in m.");
  params.addParam<FunctionName>("Tbulk", 300, "Bulk temperature of gap in K.");
  params.addParam<FunctionName>(
      "h_primary", 0.0, "Convective heat transfer coefficient (primary face) in W/m^2/K.");
  params.addParam<FunctionName>(
      "h_neighbor", 0.0, "Convective heat transfer coefficient (neighbor face) in W/m^2/K.");
  params.addParam<FunctionName>("emissivity_primary", 0.0, "Primary face emissivity.");
  params.addParam<FunctionName>("emissivity_neighbor", 0.0, "Neighbor face emissivity.");
  params.addParam<FunctionName>("reflectivity_primary",
                                "Primary face reflectivity, uses (1-emissivity) if not provided.");
  params.addParam<FunctionName>("reflectivity_neighbor",
                                "Neighbor face reflectivity, uses (1-emissivity) if not provided.");
  return params;
}

SideSetHeatTransferMaterial::SideSetHeatTransferMaterial(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _kgap(isParamValid("conductivity_temperature_function")
              ? getFunction("conductivity_temperature_function")
              : getFunction("conductivity")),
    _Tk(isCoupled("gap_temperature") ? &coupledValue("gap_temperature") : nullptr),
    _dgap(getFunction("gap_length")),
    _Tb(getFunction("Tbulk")),
    _hp(getFunction("h_primary")),
    _hm(getFunction("h_neighbor")),
    _eps_p(getFunction("emissivity_primary")),
    _eps_m(getFunction("emissivity_neighbor")),
    _rho_p(isParamValid("reflectivity_primary") ? &getFunction("reflectivity_primary") : nullptr),
    _rho_m(isParamValid("reflectivity_neighbor") ? &getFunction("reflectivity_neighbor") : nullptr),
    _cond(declareProperty<Real>("gap_conductance")),
    _Tbulk(declareProperty<Real>("gap_Tbulk")),
    _h_primary(declareProperty<Real>("gap_h_primary")),
    _h_neighbor(declareProperty<Real>("gap_h_neighbor")),
    _emmissivity_eff_primary(declareProperty<Real>("gap_emissivity_eff_primary")),
    _emmissivity_eff_neighbor(declareProperty<Real>("gap_emissivity_eff_neighbor")),
    _sigma(SIGMA)
{
  if ((parameters.isParamSetByUser("conductivity") ||
       isParamValid("conductivity_temperature_function")) &&
      !parameters.isParamSetByUser("gap_length"))
    paramError("gap_length", "gap_length not set, but conduction term requested.");
  if (parameters.isParamSetByUser("gap_length") &&
      !(parameters.isParamSetByUser("conductivity") ||
        isParamValid("conductivity_temperature_function")))
    paramError("conductivity", "conductivity not set, but conduction term requested.");
  if (isParamValid("conductivity_temperature_function") &&
      parameters.isParamSetByUser("conductivity"))
    paramError("conductivity",
               "Cannot specify both conductivity and conductivity_temperature_function.");
  if (isParamValid("conductivity_temperature_function") && !_Tk)
    paramError("gap_temperature",
               "Variable specification for temp needed if specifying a temperature dependent "
               "conductivity.");

  if (parameters.isParamSetByUser("h_primary") && !parameters.isParamSetByUser("h_neighbor"))
    paramError("h_neighbor", "h_neighbor not set, but convection term requested.");
  if (parameters.isParamSetByUser("h_neighbor") && !parameters.isParamSetByUser("h_primary"))
    paramError("h_primary", "h_primary not set, but convection term requested.");

  if (parameters.isParamSetByUser("emissivity_primary") &&
      !parameters.isParamSetByUser("emissivity_neighbor"))
    paramError("emissivity_neighbor", "emissivity_neighbor not set, but radiation term requested");
  if (parameters.isParamSetByUser("emissivity_neighbor") &&
      !parameters.isParamSetByUser("emissivity_primary"))
    paramError("emissivity_primary", "emissivity_primary not set, but radiation term requested");
}

void
SideSetHeatTransferMaterial::computeQpProperties()
{
  // Conductance defined as k_{gap}/\delta
  Real tqp = (_Tk ? (*_Tk)[_qp] : _t);
  _cond[_qp] = _kgap.value(tqp, _q_point[_qp]) / _dgap.value(_t, _q_point[_qp]);

  // Convection parameters
  _Tbulk[_qp] = _Tb.value(_t, _q_point[_qp]);
  _h_primary[_qp] = _hp.value(_t, _q_point[_qp]);
  _h_neighbor[_qp] = _hm.value(_t, _q_point[_qp]);

  // If reflectivity not provided, assume 1 - epsilon
  Real rhop = (_rho_p ? (*_rho_p).value(_t, _q_point[_qp]) : 1.0 - _eps_p.value(_t, _q_point[_qp]));
  Real rhom = (_rho_m ? (*_rho_m).value(_t, _q_point[_qp]) : 1.0 - _eps_m.value(_t, _q_point[_qp]));

  // Compute effective emissivity (primary): \frac{\sigma\epsilon^+(1-\rho^-)}{1-rho^+\rho^-}
  _emmissivity_eff_primary[_qp] = _sigma * _eps_p.value(_t, _q_point[_qp]) * (1.0 - rhom);
  if (_emmissivity_eff_primary[_qp] != 0.0) // Making sure we don't devide by zero
    _emmissivity_eff_primary[_qp] /= 1.0 - rhop * rhom;

  // Compute effective emissivity (neighbor): \frac{\sigma\epsilon^-(1-\rho^+)}{1-rho^+\rho^-}
  _emmissivity_eff_neighbor[_qp] = _sigma * _eps_m.value(_t, _q_point[_qp]) * (1.0 - rhop);
  if (_emmissivity_eff_neighbor[_qp] != 0.0) // Making sure we don't devide by zero
    _emmissivity_eff_neighbor[_qp] /= 1.0 - rhop * rhom;
}
