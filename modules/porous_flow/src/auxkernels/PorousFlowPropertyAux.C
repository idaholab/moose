//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPropertyAux.h"

registerMooseObject("PorousFlowApp", PorousFlowPropertyAux);
registerMooseObject("PorousFlowApp", ADPorousFlowPropertyAux);

template <bool is_ad>
InputParameters
PorousFlowPropertyAuxTempl<is_ad>::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  MooseEnum property_enum("pressure saturation temperature density viscosity mass_fraction relperm "
                          "capillary_pressure enthalpy internal_energy secondary_concentration "
                          "mineral_concentration mineral_reaction_rate porosity permeability "
                          "hysteresis_order hysteresis_saturation_turning_point hysteretic_info");
  params.addRequiredParam<MooseEnum>(
      "property", property_enum, "The fluid property that this auxillary kernel is to calculate");
  params.addParam<unsigned int>("phase", 0, "The index of the phase this auxillary kernel acts on");
  params.addParam<unsigned int>(
      "liquid_phase", 0, "The index of the liquid phase (used for capillary pressure)");
  params.addParam<unsigned int>(
      "gas_phase", 1, "The index of the gas phase (used for capillary pressure)");
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index of the fluid component this auxillary kernel acts on");
  params.addParam<unsigned int>("secondary_species", 0, "The secondary chemical species number");
  params.addParam<unsigned int>("mineral_species", 0, "The mineral chemical species number");
  params.addParam<unsigned int>(
      "hysteresis_turning_point", 0, "The hysteresis turning point number");
  params.addRangeCheckedParam<unsigned int>(
      "row", 0, "row>=0 & row<=2", "Row of permeability tensor to output");
  params.addRangeCheckedParam<unsigned int>(
      "column", 0, "column>=0 & column<=2", "Column of permeability tensor to output");
  params.addClassDescription("AuxKernel to provide access to properties evaluated at quadpoints. "
                             "Note that elemental AuxVariables must be used, so that these "
                             "properties are integrated over each element.");
  return params;
}

template <bool is_ad>
PorousFlowPropertyAuxTempl<is_ad>::PorousFlowPropertyAuxTempl(const InputParameters & parameters)
  : AuxKernel(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _property_enum(getParam<MooseEnum>("property").template getEnum<PropertyEnum>()),
    _phase(getParam<unsigned int>("phase")),
    _liquid_phase(getParam<unsigned int>("liquid_phase")),
    _gas_phase(getParam<unsigned int>("gas_phase")),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _secondary_species(getParam<unsigned int>("secondary_species")),
    _mineral_species(getParam<unsigned int>("mineral_species")),
    _hysteresis_turning_point(getParam<unsigned int>("hysteresis_turning_point")),
    _k_row(getParam<unsigned int>("row")),
    _k_col(getParam<unsigned int>("column"))
{
  // Check that the phase and fluid_component are valid
  if (_phase >= _dictator.numPhases())
    paramError("phase",
               "Phase number entered is greater than the number of phases specified in the "
               "Dictator. Remember that indexing starts at 0");

  if (_fluid_component >= _dictator.numComponents())
    paramError("fluid_component",
               "Fluid component number entered is greater than the number of fluid components "
               "specified in the Dictator. Remember that indexing starts at 0");

  // Check the parameters used to calculate capillary pressure
  if (_property_enum == PropertyEnum::CAPILLARY_PRESSURE)
  {
    if (_liquid_phase >= _dictator.numPhases())
      paramError(
          "liquid_phase",
          "Liquid phase number entered is greater than the number of phases specified in the "
          "Dictator. Remember that indexing starts at 0");

    if (_gas_phase >= _dictator.numPhases())
      paramError("gas_phase",
                 "Gas phase number entered is greater than the number of phases specified in the "
                 "Dictator. Remember that indexing starts at 0");

    if (_liquid_phase == _gas_phase)
      paramError("liquid_phase", "Liquid phase number entered cannot be equal to gas_phase");
  }

  if (_property_enum == PropertyEnum::SECONDARY_CONCENTRATION &&
      (_secondary_species >= _dictator.numAqueousEquilibrium()))
    paramError("secondary_species",
               "Secondary species number entered is greater than the number of aqueous equilibrium "
               "chemical reactions specified in the Dictator. Remember that indexing starts at 0");

  if ((_property_enum == PropertyEnum::MINERAL_CONCENTRATION ||
       _property_enum == PropertyEnum::MINERAL_REACTION_RATE) &&
      (_mineral_species >= _dictator.numAqueousKinetic()))
    paramError("mineral_species",
               "Mineral species number entered is greater than the number of aqueous "
               "precipitation-dissolution chemical reactions specified in the Dictator. Remember "
               "that indexing starts at 0");

  if (_hysteresis_turning_point >= PorousFlowConstants::MAX_HYSTERESIS_ORDER)
    paramError("hysteresis_turning_point",
               "The maximum number of hysteresis turning points is ",
               PorousFlowConstants::MAX_HYSTERESIS_ORDER);

  // Only get material properties required by this instance of the AuxKernel
  switch (_property_enum)
  {
    case PropertyEnum::PRESSURE:
      _pressure =
          &getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_porepressure_qp");
      break;

    case PropertyEnum::SATURATION:
      _saturation =
          &getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_saturation_qp");
      break;

    case PropertyEnum::TEMPERATURE:
      _temperature = &getGenericMaterialProperty<Real, is_ad>("PorousFlow_temperature_qp");
      break;

    case PropertyEnum::DENSITY:
      _fluid_density = &getGenericMaterialProperty<std::vector<Real>, is_ad>(
          "PorousFlow_fluid_phase_density_qp");
      break;

    case PropertyEnum::VISCOSITY:
      _fluid_viscosity =
          &getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_viscosity_qp");
      break;

    case PropertyEnum::MASS_FRACTION:
      _mass_fractions = &getGenericMaterialProperty<std::vector<std::vector<Real>>, is_ad>(
          "PorousFlow_mass_frac_qp");
      break;

    case PropertyEnum::RELPERM:
      _relative_permeability = &getGenericMaterialProperty<std::vector<Real>, is_ad>(
          "PorousFlow_relative_permeability_qp");
      break;

    case PropertyEnum::CAPILLARY_PRESSURE:
      _pressure =
          &getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_porepressure_qp");
      break;

    case PropertyEnum::ENTHALPY:
      _enthalpy = &getGenericMaterialProperty<std::vector<Real>, is_ad>(
          "PorousFlow_fluid_phase_enthalpy_qp");
      break;

    case PropertyEnum::INTERNAL_ENERGY:
      _internal_energy = &getGenericMaterialProperty<std::vector<Real>, is_ad>(
          "PorousFlow_fluid_phase_internal_energy_qp");
      break;

    case PropertyEnum::SECONDARY_CONCENTRATION:
      _sec_conc = &getGenericMaterialProperty<std::vector<Real>, is_ad>(
          "PorousFlow_secondary_concentration_qp");
      break;

    case PropertyEnum::MINERAL_CONCENTRATION:
      _mineral_conc = &getGenericMaterialProperty<std::vector<Real>, is_ad>(
          "PorousFlow_mineral_concentration_qp");
      break;

    case PropertyEnum::MINERAL_REACTION_RATE:
      _mineral_reaction_rate = &getGenericMaterialProperty<std::vector<Real>, is_ad>(
          "PorousFlow_mineral_reaction_rate_qp");
      break;

    case PropertyEnum::POROSITY:
      _porosity = &getGenericMaterialProperty<Real, is_ad>("PorousFlow_porosity_qp");
      break;

    case PropertyEnum::PERMEABILITY:
      _permeability =
          &getGenericMaterialProperty<RealTensorValue, is_ad>("PorousFlow_permeability_qp");
      break;

    case PropertyEnum::HYSTERESIS_ORDER:
      _hys_order = &getMaterialProperty<unsigned int>("PorousFlow_hysteresis_order_qp");
      break;

    case PropertyEnum::HYSTERESIS_SATURATION_TURNING_POINT:
      _hys_sat_tps =
          &getMaterialProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
              "PorousFlow_hysteresis_saturation_tps_qp");
      break;

    case PropertyEnum::HYSTERETIC_INFO:
      _hys_info = &getMaterialProperty<Real>("PorousFlow_hysteretic_info_qp");
      break;
  }
}

template <bool is_ad>
Real
PorousFlowPropertyAuxTempl<is_ad>::computeValue()
{
  Real property = 0.0;

  switch (_property_enum)
  {
    case PropertyEnum::PRESSURE:
      property = MetaPhysicL::raw_value((*_pressure)[_qp][_phase]);
      break;

    case PropertyEnum::SATURATION:
      property = MetaPhysicL::raw_value((*_saturation)[_qp][_phase]);
      break;

    case PropertyEnum::TEMPERATURE:
      property = MetaPhysicL::raw_value((*_temperature)[_qp]);
      break;

    case PropertyEnum::DENSITY:
      property = MetaPhysicL::raw_value((*_fluid_density)[_qp][_phase]);
      break;

    case PropertyEnum::VISCOSITY:
      property = MetaPhysicL::raw_value((*_fluid_viscosity)[_qp][_phase]);
      break;

    case PropertyEnum::MASS_FRACTION:
      property = MetaPhysicL::raw_value((*_mass_fractions)[_qp][_phase][_fluid_component]);
      break;

    case PropertyEnum::RELPERM:
      property = MetaPhysicL::raw_value((*_relative_permeability)[_qp][_phase]);
      break;

    case PropertyEnum::CAPILLARY_PRESSURE:
      property =
          MetaPhysicL::raw_value((*_pressure)[_qp][_gas_phase] - (*_pressure)[_qp][_liquid_phase]);
      break;

    case PropertyEnum::ENTHALPY:
      property = MetaPhysicL::raw_value((*_enthalpy)[_qp][_phase]);
      break;

    case PropertyEnum::INTERNAL_ENERGY:
      property = MetaPhysicL::raw_value((*_internal_energy)[_qp][_phase]);
      break;

    case PropertyEnum::SECONDARY_CONCENTRATION:
      property = MetaPhysicL::raw_value((*_sec_conc)[_qp][_secondary_species]);
      break;

    case PropertyEnum::MINERAL_CONCENTRATION:
      property = MetaPhysicL::raw_value((*_mineral_conc)[_qp][_mineral_species]);
      break;

    case PropertyEnum::MINERAL_REACTION_RATE:
      property = MetaPhysicL::raw_value((*_mineral_reaction_rate)[_qp][_mineral_species]);
      break;

    case PropertyEnum::POROSITY:
      property = MetaPhysicL::raw_value((*_porosity)[_qp]);
      break;

    case PropertyEnum::PERMEABILITY:
      property = MetaPhysicL::raw_value((*_permeability)[_qp](_k_row, _k_col));
      break;

    case PropertyEnum::HYSTERESIS_ORDER:
      property = (*_hys_order)[_qp];
      break;

    case PropertyEnum::HYSTERESIS_SATURATION_TURNING_POINT:
      property = (*_hys_sat_tps)[_qp].at(_hysteresis_turning_point);
      break;

    case PropertyEnum::HYSTERETIC_INFO:
      property = (*_hys_info)[_qp];
      break;
  }

  return property;
}

template class PorousFlowPropertyAuxTempl<false>;
template class PorousFlowPropertyAuxTempl<true>;
