//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateBaseMaterial.h"
#include "PorousFlowCapillaryPressure.h"

template <bool is_ad>
InputParameters
PorousFlowFluidStateBaseMaterialTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowVariableBase::validParams();
  MooseEnum unit_choice("Kelvin=0 Celsius=1", "Kelvin");
  params.addParam<MooseEnum>(
      "temperature_unit", unit_choice, "The unit of the temperature variable");
  params.addRequiredParam<UserObjectName>("capillary_pressure",
                                          "Name of the UserObject defining the capillary pressure");
  params.addPrivateParam<std::string>("pf_material_type", "fluid_state");
  params.addClassDescription("Base class for fluid state calculations using persistent primary "
                             "variables and a vapor-liquid flash");
  return params;
}

template <bool is_ad>
PorousFlowFluidStateBaseMaterialTempl<is_ad>::PorousFlowFluidStateBaseMaterialTempl(
    const InputParameters & parameters)
  : PorousFlowVariableBaseTempl<is_ad>(parameters),
    _T_c2k(this->template getParam<MooseEnum>("temperature_unit") == 0 ? 0.0 : 273.15),
    _is_initqp(false),
    _pc(this->template getUserObject<PorousFlowCapillaryPressure>("capillary_pressure")),
    _sfx(_nodal_material ? "_nodal" : "_qp"),
    _mass_frac(this->template declareGenericProperty<std::vector<std::vector<Real>>, is_ad>(
        "PorousFlow_mass_frac" + _sfx)),
    _grad_mass_frac_qp(
        _nodal_material
            ? nullptr
            : &this->template declareGenericProperty<std::vector<std::vector<RealGradient>>, is_ad>(
                  "PorousFlow_grad_mass_frac" + _sfx)),
    _dmass_frac_dvar(
        is_ad ? nullptr
              : &this->template declareProperty<std::vector<std::vector<std::vector<Real>>>>(
                    "dPorousFlow_mass_frac" + _sfx + "_dvar")),
    _fluid_density(this->template declareGenericProperty<std::vector<Real>, is_ad>(
        "PorousFlow_fluid_phase_density" + _sfx)),
    _dfluid_density_dvar(is_ad ? nullptr
                               : &this->template declareProperty<std::vector<std::vector<Real>>>(
                                     "dPorousFlow_fluid_phase_density" + _sfx + "_dvar")),
    _fluid_viscosity(this->template declareGenericProperty<std::vector<Real>, is_ad>(
        "PorousFlow_viscosity" + _sfx)),
    _dfluid_viscosity_dvar(is_ad ? nullptr
                                 : &this->template declareProperty<std::vector<std::vector<Real>>>(
                                       "dPorousFlow_viscosity" + _sfx + "_dvar")),
    _fluid_enthalpy(this->template declareGenericProperty<std::vector<Real>, is_ad>(
        "PorousFlow_fluid_phase_enthalpy" + _sfx)),
    _dfluid_enthalpy_dvar(is_ad ? nullptr
                                : &this->template declareProperty<std::vector<std::vector<Real>>>(
                                      "dPorousFlow_fluid_phase_enthalpy" + _sfx + "_dvar")),
    _fluid_internal_energy(this->template declareGenericProperty<std::vector<Real>, is_ad>(
        "PorousFlow_fluid_phase_internal_energy" + _sfx)),
    _dfluid_internal_energy_dvar(
        is_ad ? nullptr
              : &this->template declareProperty<std::vector<std::vector<Real>>>(
                    "dPorousFlow_fluid_phase_internal_energy" + _sfx + "_dvar"))
{
  // Set the size of the FluidStateProperties vector
  _fsp.resize(_num_phases, FluidStateProperties(_num_components));
}

template <>
GenericReal<false>
PorousFlowFluidStateBaseMaterialTempl<false>::genericValue(const ADReal & value)
{
  return MetaPhysicL::raw_value(value);
}

template <>
GenericReal<true>
PorousFlowFluidStateBaseMaterialTempl<true>::genericValue(const ADReal & value)
{
  return value;
}

template <bool is_ad>
void
PorousFlowFluidStateBaseMaterialTempl<is_ad>::initQpStatefulProperties()
{
  _is_initqp = true;
  // Set the size of pressure and saturation vectors
  PorousFlowVariableBaseTempl<is_ad>::initQpStatefulProperties();

  // Set the size of all other vectors
  setMaterialVectorSize();

  thermophysicalProperties();

  // Set the initial values of the properties at the nodes.
  // Note: not required for qp materials as no old values at the qps are requested
  // unless the material is AD (for the FV case there are no nodal materials)
  if (_nodal_material || is_ad)
    for (unsigned int ph = 0; ph < _num_phases; ++ph)
    {
      _saturation[_qp][ph] = genericValue(_fsp[ph].saturation);
      _porepressure[_qp][ph] = genericValue(_fsp[ph].pressure);
      _fluid_density[_qp][ph] = genericValue(_fsp[ph].density);
      _fluid_viscosity[_qp][ph] = genericValue(_fsp[ph].viscosity);
      _fluid_enthalpy[_qp][ph] = genericValue(_fsp[ph].enthalpy);
      _fluid_internal_energy[_qp][ph] = genericValue(_fsp[ph].internal_energy);

      for (unsigned int comp = 0; comp < _num_components; ++comp)
        _mass_frac[_qp][ph][comp] = genericValue(_fsp[ph].mass_fraction[comp]);
    }
}

template <bool is_ad>
void
PorousFlowFluidStateBaseMaterialTempl<is_ad>::computeQpProperties()
{
  _is_initqp = false;
  // Size pressure and saturation and prepare the derivative vectors
  PorousFlowVariableBaseTempl<is_ad>::computeQpProperties();

  // Set the size of all other vectors
  setMaterialVectorSize();

  // Calculate all required thermophysical properties
  thermophysicalProperties();

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _saturation[_qp][ph] = genericValue(_fsp[ph].saturation);
    _porepressure[_qp][ph] = genericValue(_fsp[ph].pressure);
    _fluid_density[_qp][ph] = genericValue(_fsp[ph].density);
    _fluid_viscosity[_qp][ph] = genericValue(_fsp[ph].viscosity);
    _fluid_enthalpy[_qp][ph] = genericValue(_fsp[ph].enthalpy);
    _fluid_internal_energy[_qp][ph] = genericValue(_fsp[ph].internal_energy);

    for (unsigned int comp = 0; comp < _num_components; ++comp)
      _mass_frac[_qp][ph][comp] = genericValue(_fsp[ph].mass_fraction[comp]);
  }
}

template <bool is_ad>
void
PorousFlowFluidStateBaseMaterialTempl<is_ad>::setMaterialVectorSize() const
{
  _fluid_density[_qp].resize(_num_phases, 0.0);
  _fluid_viscosity[_qp].resize(_num_phases, 0.0);
  _fluid_enthalpy[_qp].resize(_num_phases, 0.0);
  _fluid_internal_energy[_qp].resize(_num_phases, 0.0);
  _mass_frac[_qp].resize(_num_phases);

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _mass_frac[_qp][ph].resize(_num_components);

  // Derivatives and gradients are not required in initQpStatefulProperties
  if (!_is_initqp)
  {
    // The gradient of the mass fractions are needed for qp materials and AD materials
    if (!_nodal_material || is_ad)
    {
      (*_grad_mass_frac_qp)[_qp].resize(_num_phases);

      for (unsigned int ph = 0; ph < _num_phases; ++ph)
        (*_grad_mass_frac_qp)[_qp][ph].assign(_num_components, RealGradient());
    }

    // No derivatives are required for AD materials
    if (!is_ad)
    {
      (*_dfluid_density_dvar)[_qp].resize(_num_phases);
      (*_dfluid_viscosity_dvar)[_qp].resize(_num_phases);
      (*_dfluid_enthalpy_dvar)[_qp].resize(_num_phases);
      (*_dfluid_internal_energy_dvar)[_qp].resize(_num_phases);
      (*_dmass_frac_dvar)[_qp].resize(_num_phases);

      for (unsigned int ph = 0; ph < _num_phases; ++ph)
      {
        (*_dfluid_density_dvar)[_qp][ph].assign(_num_pf_vars, 0.0);
        (*_dfluid_viscosity_dvar)[_qp][ph].assign(_num_pf_vars, 0.0);
        (*_dfluid_enthalpy_dvar)[_qp][ph].assign(_num_pf_vars, 0.0);
        (*_dfluid_internal_energy_dvar)[_qp][ph].assign(_num_pf_vars, 0.0);
        (*_dmass_frac_dvar)[_qp][ph].resize(_num_components);

        for (unsigned int comp = 0; comp < _num_components; ++comp)
          (*_dmass_frac_dvar)[_qp][ph][comp].assign(_num_pf_vars, 0.0);
      }
    }
  }
}

template class PorousFlowFluidStateBaseMaterialTempl<false>;
template class PorousFlowFluidStateBaseMaterialTempl<true>;
