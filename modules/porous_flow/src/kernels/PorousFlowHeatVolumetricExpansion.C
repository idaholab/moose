//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowHeatVolumetricExpansion.h"

#include "MooseVariable.h"

#include "libmesh/quadrature.h"

registerMooseObject("PorousFlowApp", PorousFlowHeatVolumetricExpansion);
registerMooseObject("PorousFlowApp", ADPorousFlowHeatVolumetricExpansion);

template <bool is_ad>
InputParameters
PorousFlowHeatVolumetricExpansionTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
  params.addParam<bool>("strain_at_nearest_qp",
                        false,
                        "When calculating nodal porosity that depends on strain, use the strain at "
                        "the nearest quadpoint.  This adds a small extra computational burden, and "
                        "is not necessary for simulations involving only linear lagrange elements. "
                        " If you set this to true, you will also want to set the same parameter to "
                        "true for related Kernels and Materials");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.set<bool>("use_displaced_mesh") = false;
  params.suppressParameter<bool>("use_displaced_mesh");
  params.addClassDescription("Energy-density*rate_of_solid_volumetric_expansion.  The "
                             "energy-density is lumped to the nodes");
  return params;
}

template <bool is_ad>
PorousFlowHeatVolumetricExpansionTempl<is_ad>::PorousFlowHeatVolumetricExpansionTempl(
    const InputParameters & parameters)
  : PorousFlowLumpedKernelBaseTempl<is_ad>(parameters),
    _dictator(this->template getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _var_is_porflow_var(_dictator.isPorousFlowVariable(_var.number())),
    _num_phases(_dictator.numPhases()),
    _fluid_present(_num_phases > 0),
    _strain_at_nearest_qp(this->template getParam<bool>("strain_at_nearest_qp")),
    _porosity(this->template getGenericMaterialProperty<Real, is_ad>("PorousFlow_porosity_nodal")),
    _dporosity_dvar(is_ad ? nullptr
                          : &this->template getMaterialProperty<std::vector<Real>>(
                                "dPorousFlow_porosity_nodal_dvar")),
    _dporosity_dgradvar(is_ad ? nullptr
                              : &this->template getMaterialProperty<std::vector<RealGradient>>(
                                    "dPorousFlow_porosity_nodal_dgradvar")),
    _nearest_qp(_strain_at_nearest_qp ? &this->template getMaterialProperty<unsigned int>(
                                            "PorousFlow_nearestqp_nodal")
                                      : nullptr),
    _rock_energy_nodal(this->template getGenericMaterialProperty<Real, is_ad>(
        "PorousFlow_matrix_internal_energy_nodal")),
    _drock_energy_nodal_dvar(is_ad ? nullptr
                                   : &this->template getMaterialProperty<std::vector<Real>>(
                                         "dPorousFlow_matrix_internal_energy_nodal_dvar")),
    _fluid_density(_fluid_present
                       ? &this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
                             "PorousFlow_fluid_phase_density_nodal")
                       : nullptr),
    _dfluid_density_dvar(_fluid_present && !is_ad
                             ? &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                                   "dPorousFlow_fluid_phase_density_nodal_dvar")
                             : nullptr),
    _fluid_saturation_nodal(
        _fluid_present ? &this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
                             "PorousFlow_saturation_nodal")
                       : nullptr),
    _dfluid_saturation_nodal_dvar(
        _fluid_present && !is_ad
            ? &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                  "dPorousFlow_saturation_nodal_dvar")
            : nullptr),
    _energy_nodal(_fluid_present
                      ? &this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
                            "PorousFlow_fluid_phase_internal_energy_nodal")
                      : nullptr),
    _denergy_nodal_dvar(_fluid_present && !is_ad
                            ? &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                                  "dPorousFlow_fluid_phase_internal_energy_nodal_dvar")
                            : nullptr),
    _strain_rate_qp(this->template getGenericMaterialProperty<Real, is_ad>(
        "PorousFlow_volumetric_strain_rate_qp")),
    _dstrain_rate_qp_dvar(is_ad ? nullptr
                                : &this->template getMaterialProperty<std::vector<RealGradient>>(
                                      "dPorousFlow_volumetric_strain_rate_qp_dvar"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowHeatVolumetricExpansionTempl<is_ad>::computeQpResidual()
{
  GenericReal<is_ad> energy = (1.0 - _porosity[_i]) * _rock_energy_nodal[_i];
  for (unsigned ph = 0; ph < _num_phases; ++ph)
    energy += (*_fluid_density)[_i][ph] * (*_fluid_saturation_nodal)[_i][ph] *
              (*_energy_nodal)[_i][ph] * _porosity[_i];

  return _test[_i][_qp] * energy * _strain_rate_qp[_qp];
}

template <bool is_ad>
Real
PorousFlowHeatVolumetricExpansionTempl<is_ad>::computeQpJacobian()
{
  return computedEnergyQpJac(_var.number()) + computedVolQpJac(_var.number());
}

template <bool is_ad>
Real
PorousFlowHeatVolumetricExpansionTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  return computedEnergyQpJac(jvar) + computedVolQpJac(jvar);
}

template <bool is_ad>
Real
PorousFlowHeatVolumetricExpansionTempl<is_ad>::computedVolQpJac(unsigned int jvar)
{
  if constexpr (!is_ad)
  {
    if (_dictator.notPorousFlowVariable(jvar))
      return 0.0;

    Real energy = (1.0 - _porosity[_i]) * _rock_energy_nodal[_i];
    for (unsigned ph = 0; ph < _num_phases; ++ph)
      energy += (*_fluid_density)[_i][ph] * (*_fluid_saturation_nodal)[_i][ph] *
                (*_energy_nodal)[_i][ph] * _porosity[_i];

    const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);
    Real dvol = (*_dstrain_rate_qp_dvar)[_qp][pvar] * _grad_phi[_j][_qp];

    return _test[_i][_qp] * energy * dvol;
  }
  else
  {
    libmesh_ignore(jvar);
    return 0.0;
  }
}

template <bool is_ad>
Real
PorousFlowHeatVolumetricExpansionTempl<is_ad>::computedEnergyQpJac(unsigned int jvar)
{
  if constexpr (!is_ad)
  {
    if (_dictator.notPorousFlowVariable(jvar))
      return 0.0;

    const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);
    const unsigned nearest_qp = (_strain_at_nearest_qp ? (*_nearest_qp)[_i] : _i);

    Real denergy = -(*_dporosity_dgradvar)[_i][pvar] * _grad_phi[_j][_i] * _rock_energy_nodal[_i];
    for (unsigned ph = 0; ph < _num_phases; ++ph)
      denergy += (*_fluid_density)[_i][ph] * (*_fluid_saturation_nodal)[_i][ph] *
                 (*_energy_nodal)[_i][ph] * (*_dporosity_dgradvar)[_i][pvar] *
                 _grad_phi[_j][nearest_qp];

    if (_i != _j)
      return _test[_i][_qp] * denergy * _strain_rate_qp[_qp];

    denergy += (*_drock_energy_nodal_dvar)[_i][pvar] * (1.0 - _porosity[_i]);
    denergy -= _rock_energy_nodal[_i] * (*_dporosity_dvar)[_i][pvar];
    for (unsigned ph = 0; ph < _num_phases; ++ph)
    {
      denergy += (*_dfluid_density_dvar)[_i][ph][pvar] * (*_fluid_saturation_nodal)[_i][ph] *
                 (*_energy_nodal)[_i][ph] * _porosity[_i];
      denergy += (*_fluid_density)[_i][ph] * (*_dfluid_saturation_nodal_dvar)[_i][ph][pvar] *
                 (*_energy_nodal)[_i][ph] * _porosity[_i];
      denergy += (*_fluid_density)[_i][ph] * (*_fluid_saturation_nodal)[_i][ph] *
                 (*_denergy_nodal_dvar)[_i][ph][pvar] * _porosity[_i];
      denergy += (*_fluid_density)[_i][ph] * (*_fluid_saturation_nodal)[_i][ph] *
                 (*_energy_nodal)[_i][ph] * (*_dporosity_dvar)[_i][pvar];
    }

    return _test[_i][_qp] * denergy * _strain_rate_qp[_qp];
  }
  else
  {
    libmesh_ignore(jvar);
    return 0.0;
  }
}

template class PorousFlowHeatVolumetricExpansionTempl<false>;
template class PorousFlowHeatVolumetricExpansionTempl<true>;
