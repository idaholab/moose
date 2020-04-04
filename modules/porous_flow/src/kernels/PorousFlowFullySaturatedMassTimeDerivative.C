//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFullySaturatedMassTimeDerivative.h"

#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowFullySaturatedMassTimeDerivative);

InputParameters
PorousFlowFullySaturatedMassTimeDerivative::validParams()
{
  InputParameters params = TimeKernel::validParams();
  MooseEnum coupling_type("Hydro ThermoHydro HydroMechanical ThermoHydroMechanical", "Hydro");
  params.addParam<MooseEnum>("coupling_type",
                             coupling_type,
                             "The type of simulation.  For simulations involving Mechanical "
                             "deformations, you will need to supply the correct Biot coefficient.  "
                             "For simulations involving Thermal flows, you will need an associated "
                             "ConstantThermalExpansionCoefficient Material");
  params.addRangeCheckedParam<Real>(
      "biot_coefficient", 1.0, "biot_coefficient>=0 & biot_coefficient<=1", "Biot coefficient");
  params.addParam<bool>("multiply_by_density",
                        true,
                        "If true, then this Kernel is the time derivative of the fluid "
                        "mass.  If false, then this Kernel is the derivative of the "
                        "fluid volume (which is common in poro-mechanics)");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addClassDescription("Fully-saturated version of the single-component, single-phase fluid "
                             "mass derivative wrt time");
  return params;
}

PorousFlowFullySaturatedMassTimeDerivative::PorousFlowFullySaturatedMassTimeDerivative(
    const InputParameters & parameters)
  : TimeKernel(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _var_is_porflow_var(_dictator.isPorousFlowVariable(_var.number())),
    _multiply_by_density(getParam<bool>("multiply_by_density")),
    _coupling_type(getParam<MooseEnum>("coupling_type").getEnum<CouplingTypeEnum>()),
    _includes_thermal(_coupling_type == CouplingTypeEnum::ThermoHydro ||
                      _coupling_type == CouplingTypeEnum::ThermoHydroMechanical),
    _includes_mechanical(_coupling_type == CouplingTypeEnum::HydroMechanical ||
                         _coupling_type == CouplingTypeEnum::ThermoHydroMechanical),
    _biot_coefficient(getParam<Real>("biot_coefficient")),
    _biot_modulus(getMaterialProperty<Real>("PorousFlow_constant_biot_modulus_qp")),
    _thermal_coeff(_includes_thermal ? &getMaterialProperty<Real>(
                                           "PorousFlow_constant_thermal_expansion_coefficient_qp")
                                     : nullptr),
    _fluid_density(_multiply_by_density ? &getMaterialProperty<std::vector<Real>>(
                                              "PorousFlow_fluid_phase_density_qp")
                                        : nullptr),
    _dfluid_density_dvar(_multiply_by_density
                             ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                   "dPorousFlow_fluid_phase_density_qp_dvar")
                             : nullptr),
    _pp(getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _pp_old(getMaterialPropertyOld<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _dpp_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_porepressure_qp_dvar")),
    _temperature(_includes_thermal ? &getMaterialProperty<Real>("PorousFlow_temperature_qp")
                                   : nullptr),
    _temperature_old(_includes_thermal ? &getMaterialPropertyOld<Real>("PorousFlow_temperature_qp")
                                       : nullptr),
    _dtemperature_dvar(_includes_thermal ? &getMaterialProperty<std::vector<Real>>(
                                               "dPorousFlow_temperature_qp_dvar")
                                         : nullptr),
    _strain_rate(_includes_mechanical
                     ? &getMaterialProperty<Real>("PorousFlow_volumetric_strain_rate_qp")
                     : nullptr),
    _dstrain_rate_dvar(_includes_mechanical ? &getMaterialProperty<std::vector<RealGradient>>(
                                                  "dPorousFlow_volumetric_strain_rate_qp_dvar")
                                            : nullptr)
{
  if (_dictator.numComponents() != 1 || _dictator.numPhases() != 1)
    mooseError("PorousFlowFullySaturatedMassTimeDerivative is only applicable to single-phase, "
               "single-component fluid-flow problems.  The Dictator proclaims that you have more "
               "than one phase or more than one fluid component.  The Dictator does not take such "
               "mistakes lightly");
}

Real
PorousFlowFullySaturatedMassTimeDerivative::computeQpResidual()
{
  const unsigned phase = 0;
  Real volume = (_pp[_qp][phase] - _pp_old[_qp][phase]) / _dt / _biot_modulus[_qp];
  if (_includes_thermal)
    volume -= (*_thermal_coeff)[_qp] * ((*_temperature)[_qp] - (*_temperature_old)[_qp]) / _dt;
  if (_includes_mechanical)
    volume += _biot_coefficient * (*_strain_rate)[_qp];
  if (_multiply_by_density)
    return _test[_i][_qp] * (*_fluid_density)[_qp][phase] * volume;
  return _test[_i][_qp] * volume;
}

Real
PorousFlowFullySaturatedMassTimeDerivative::computeQpJacobian()
{
  // If the variable is not a PorousFlow variable (very unusual), the diag Jacobian terms are 0
  if (!_var_is_porflow_var)
    return 0.0;
  return computeQpJac(_dictator.porousFlowVariableNum(_var.number()));
}

Real
PorousFlowFullySaturatedMassTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  // If the variable is not a PorousFlow variable, the OffDiag Jacobian terms are 0
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;
  return computeQpJac(_dictator.porousFlowVariableNum(jvar));
}

Real
PorousFlowFullySaturatedMassTimeDerivative::computeQpJac(unsigned int pvar)
{
  const unsigned phase = 0;
  Real volume = (_pp[_qp][phase] - _pp_old[_qp][phase]) / _dt / _biot_modulus[_qp];
  Real dvolume = _dpp_dvar[_qp][phase][pvar] / _dt / _biot_modulus[_qp] * _phi[_j][_qp];
  if (_includes_thermal)
  {
    volume -= (*_thermal_coeff)[_qp] * ((*_temperature)[_qp] - (*_temperature_old)[_qp]) / _dt;
    dvolume -= (*_thermal_coeff)[_qp] * (*_dtemperature_dvar)[_qp][pvar] / _dt * _phi[_j][_qp];
  }
  if (_includes_mechanical)
  {
    volume += _biot_coefficient * (*_strain_rate)[_qp];
    dvolume += _biot_coefficient * (*_dstrain_rate_dvar)[_qp][pvar] * _grad_phi[_j][_qp];
  }
  if (_multiply_by_density)
    return _test[_i][_qp] * ((*_fluid_density)[_qp][phase] * dvolume +
                             (*_dfluid_density_dvar)[_qp][phase][pvar] * _phi[_j][_qp] * volume);
  return _test[_i][_qp] * dvolume;
}
