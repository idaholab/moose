//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidPropertiesBase.h"

template <bool is_ad>
InputParameters
PorousFlowFluidPropertiesBaseTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMaterialBase::validParams();
  MooseEnum unit_choice("Kelvin=0 Celsius=1", "Kelvin");
  params.addParam<MooseEnum>(
      "temperature_unit", unit_choice, "The unit of the temperature variable");
  MooseEnum p_unit_choice("Pa MPa", "Pa");
  params.addParam<MooseEnum>("pressure_unit",
                             p_unit_choice,
                             "The unit of the pressure variable used everywhere in the input file "
                             "except for in the FluidProperties-module objects");
  MooseEnum time_unit_choice("seconds hours days years", "seconds");
  params.addParam<MooseEnum>("time_unit",
                             time_unit_choice,
                             "The unit of time used everywhere in the input file except for in the "
                             "FluidProperties-module objects");
  params.addParam<bool>(
      "compute_density_and_viscosity", true, "Compute the fluid density and viscosity");
  params.addParam<bool>("compute_internal_energy", true, "Compute the fluid internal energy");
  params.addParam<bool>("compute_enthalpy", true, "Compute the fluid enthalpy");
  params.addPrivateParam<std::string>("pf_material_type", "fluid_properties");
  params.addPrivateParam<bool>("is_ad", is_ad);
  params.addClassDescription("Base class for PorousFlow fluid materials");
  return params;
}

template <bool is_ad>
PorousFlowFluidPropertiesBaseTempl<is_ad>::PorousFlowFluidPropertiesBaseTempl(
    const InputParameters & parameters)
  : PorousFlowMaterialBase(parameters),
    _porepressure(
        _nodal_material
            ? getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_porepressure_nodal")
            : getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_porepressure_qp")),
    _temperature(_nodal_material
                     ? getGenericMaterialProperty<Real, is_ad>("PorousFlow_temperature_nodal")
                     : getGenericMaterialProperty<Real, is_ad>("PorousFlow_temperature_qp")),
    _t_c2k(getParam<MooseEnum>("temperature_unit") == 0 ? 0.0 : 273.15),
    _R(8.3144598),
    _p_unit(
        this->template getParam<MooseEnum>("pressure_unit").template getEnum<PressureUnitEnum>()),
    _pressure_to_Pascals(_p_unit == PressureUnitEnum::Pa ? 1.0 : 1.0E6),
    _time_unit(this->template getParam<MooseEnum>("time_unit").template getEnum<TimeUnitEnum>()),
    _time_to_seconds(_time_unit == TimeUnitEnum::seconds ? 1.0
                     : _time_unit == TimeUnitEnum::hours ? 3600.0
                     : _time_unit == TimeUnitEnum::days  ? 3600.0 * 24
                                                         : 3600 * 24 * 365.25),
    _compute_rho_mu(this->template getParam<bool>("compute_density_and_viscosity")),
    _compute_internal_energy(this->template getParam<bool>("compute_internal_energy")),
    _compute_enthalpy(this->template getParam<bool>("compute_enthalpy")),
    _density(_compute_rho_mu
                 ? (_nodal_material ? &this->template declareGenericProperty<Real, is_ad>(
                                          "PorousFlow_fluid_phase_density_nodal" + _phase)
                                    : &this->template declareGenericProperty<Real, is_ad>(
                                          "PorousFlow_fluid_phase_density_qp" + _phase))
                 : nullptr),
    _ddensity_dp(
        (_compute_rho_mu && !is_ad)
            ? (_nodal_material
                   ? &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_fluid_phase_density_nodal" + _phase, _pressure_variable_name)
                   : &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_fluid_phase_density_qp" + _phase, _pressure_variable_name))
            : nullptr),
    _ddensity_dT((_compute_rho_mu && !is_ad)
                     ? (_nodal_material ? &this->template declarePropertyDerivative<Real>(
                                              "PorousFlow_fluid_phase_density_nodal" + _phase,
                                              _temperature_variable_name)
                                        : &this->template declarePropertyDerivative<Real>(
                                              "PorousFlow_fluid_phase_density_qp" + _phase,
                                              _temperature_variable_name))
                     : nullptr),

    _viscosity(_compute_rho_mu
                   ? (_nodal_material ? &this->template declareGenericProperty<Real, is_ad>(
                                            "PorousFlow_viscosity_nodal" + _phase)
                                      : &this->template declareGenericProperty<Real, is_ad>(
                                            "PorousFlow_viscosity_qp" + _phase))
                   : nullptr),
    _dviscosity_dp((_compute_rho_mu && !is_ad)
                       ? (_nodal_material
                              ? &this->template declarePropertyDerivative<Real>(
                                    "PorousFlow_viscosity_nodal" + _phase, _pressure_variable_name)
                              : &this->template declarePropertyDerivative<Real>(
                                    "PorousFlow_viscosity_qp" + _phase, _pressure_variable_name))
                       : nullptr),
    _dviscosity_dT(
        (_compute_rho_mu && !is_ad)
            ? (_nodal_material
                   ? &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_viscosity_nodal" + _phase, _temperature_variable_name)
                   : &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_viscosity_qp" + _phase, _temperature_variable_name))
            : nullptr),

    _internal_energy(_compute_internal_energy
                         ? (_nodal_material
                                ? &this->template declareGenericProperty<Real, is_ad>(
                                      "PorousFlow_fluid_phase_internal_energy_nodal" + _phase)
                                : &this->template declareGenericProperty<Real, is_ad>(
                                      "PorousFlow_fluid_phase_internal_energy_qp" + _phase))
                         : nullptr),
    _dinternal_energy_dp((_compute_internal_energy && !is_ad)
                             ? (_nodal_material
                                    ? &this->template declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_nodal" + _phase,
                                          _pressure_variable_name)
                                    : &this->template declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                          _pressure_variable_name))
                             : nullptr),
    _dinternal_energy_dT((_compute_internal_energy && !is_ad)
                             ? (_nodal_material
                                    ? &this->template declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_nodal" + _phase,
                                          _temperature_variable_name)
                                    : &this->template declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                          _temperature_variable_name))
                             : nullptr),

    _enthalpy(_compute_enthalpy
                  ? (_nodal_material ? &this->template declareGenericProperty<Real, is_ad>(
                                           "PorousFlow_fluid_phase_enthalpy_nodal" + _phase)
                                     : &this->template declareGenericProperty<Real, is_ad>(
                                           "PorousFlow_fluid_phase_enthalpy_qp" + _phase))
                  : nullptr),
    _denthalpy_dp(
        (_compute_enthalpy && !is_ad)
            ? (_nodal_material
                   ? &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_fluid_phase_enthalpy_nodal" + _phase, _pressure_variable_name)
                   : &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_fluid_phase_enthalpy_qp" + _phase, _pressure_variable_name))
            : nullptr),
    _denthalpy_dT((_compute_enthalpy && !is_ad)
                      ? (_nodal_material ? &this->template declarePropertyDerivative<Real>(
                                               "PorousFlow_fluid_phase_enthalpy_nodal" + _phase,
                                               _temperature_variable_name)
                                         : &this->template declarePropertyDerivative<Real>(
                                               "PorousFlow_fluid_phase_enthalpy_qp" + _phase,
                                               _temperature_variable_name))
                      : nullptr)
{
}

template <bool is_ad>
void
PorousFlowFluidPropertiesBaseTempl<is_ad>::computeQpProperties()
{
  mooseError("computeQpProperties() must be overriden in materials derived from "
             "PorousFlowFluidPropertiesBase");
}

template class PorousFlowFluidPropertiesBaseTempl<false>;
template class PorousFlowFluidPropertiesBaseTempl<true>;
