//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMassRadioactiveDecay.h"

#include "MooseVariable.h"

#include "libmesh/quadrature.h"

#include <limits>

registerMooseObject("PorousFlowApp", PorousFlowMassRadioactiveDecay);
registerMooseObject("PorousFlowApp", ADPorousFlowMassRadioactiveDecay);

template <bool is_ad>
InputParameters
PorousFlowMassRadioactiveDecayTempl<is_ad>::validParams()
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
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index corresponding to the fluid component for this kernel");
  params.addRequiredParam<Real>("decay_rate",
                                "The decay rate (units 1/time) for the fluid component");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addClassDescription("Radioactive decay of a fluid component");
  return params;
}

template <bool is_ad>
PorousFlowMassRadioactiveDecayTempl<is_ad>::PorousFlowMassRadioactiveDecayTempl(
    const InputParameters & parameters)
  : PorousFlowLumpedKernelBaseTempl<is_ad>(parameters),
    _decay_rate(this->template getParam<Real>("decay_rate")),
    _fluid_component(this->template getParam<unsigned int>("fluid_component")),
    _dictator(this->template getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _var_is_porflow_var(_dictator.isPorousFlowVariable(_var.number())),
    _num_phases(_dictator.numPhases()),
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
    _fluid_density(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_fluid_phase_density_nodal")),
    _dfluid_density_dvar(is_ad
                             ? nullptr
                             : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                                   "dPorousFlow_fluid_phase_density_nodal_dvar")),
    _fluid_saturation_nodal(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_saturation_nodal")),
    _dfluid_saturation_nodal_dvar(
        is_ad ? nullptr
              : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                    "dPorousFlow_saturation_nodal_dvar")),
    _mass_frac(this->template getGenericMaterialProperty<std::vector<std::vector<Real>>, is_ad>(
        "PorousFlow_mass_frac_nodal")),
    _dmass_frac_dvar(
        is_ad ? nullptr
              : &this->template getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
                    "dPorousFlow_mass_frac_nodal_dvar"))
{
  if (_fluid_component >= _dictator.numComponents())
    this->paramError(
        "fluid_component",
        "The Dictator proclaims that the maximum fluid component index in this simulation is ",
        _dictator.numComponents() - 1,
        " whereas you have used ",
        _fluid_component,
        ". Remember that indexing starts at 0. The Dictator does not take such mistakes lightly.");
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowMassRadioactiveDecayTempl<is_ad>::computeQpResidual()
{
  GenericReal<is_ad> mass = 0.0;
  for (unsigned ph = 0; ph < _num_phases; ++ph)
    mass += _fluid_density[_i][ph] * _fluid_saturation_nodal[_i][ph] *
            _mass_frac[_i][ph][_fluid_component];

  return _test[_i][_qp] * _decay_rate * _porosity[_i] * mass;
}

template <bool is_ad>
Real
PorousFlowMassRadioactiveDecayTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
  {
    if (!_var_is_porflow_var)
      return 0.0;
    return computeQpJac(_dictator.porousFlowVariableNum(_var.number()));
  }
  return 0.0;
}

template <bool is_ad>
Real
PorousFlowMassRadioactiveDecayTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  if constexpr (!is_ad)
  {
    if (_dictator.notPorousFlowVariable(jvar))
      return 0.0;
    return computeQpJac(_dictator.porousFlowVariableNum(jvar));
  }
  return 0.0;
}

template <bool is_ad>
Real
PorousFlowMassRadioactiveDecayTempl<is_ad>::computeQpJac(unsigned int pvar)
{
  if constexpr (!is_ad)
  {
    const unsigned nearest_qp = (_strain_at_nearest_qp ? (*_nearest_qp)[_i] : _i);

    // porosity can depend on grad(variables) evaluated at qps, so the grad-term is nonzero even
    // for off-node DOFs
    Real dmass = 0.0;
    for (unsigned ph = 0; ph < _num_phases; ++ph)
      dmass += _fluid_density[_i][ph] * _fluid_saturation_nodal[_i][ph] *
               _mass_frac[_i][ph][_fluid_component] * (*_dporosity_dgradvar)[_i][pvar] *
               _grad_phi[_j][nearest_qp];

    if (_i != _j)
      return _test[_i][_qp] * _decay_rate * dmass;

    // As the fluid mass is lumped to the nodes, only non-zero terms are for _i==_j
    for (unsigned ph = 0; ph < _num_phases; ++ph)
    {
      dmass += (*_dfluid_density_dvar)[_i][ph][pvar] * _fluid_saturation_nodal[_i][ph] *
               _mass_frac[_i][ph][_fluid_component] * _porosity[_i];
      dmass += _fluid_density[_i][ph] * (*_dfluid_saturation_nodal_dvar)[_i][ph][pvar] *
               _mass_frac[_i][ph][_fluid_component] * _porosity[_i];
      dmass += _fluid_density[_i][ph] * _fluid_saturation_nodal[_i][ph] *
               (*_dmass_frac_dvar)[_i][ph][_fluid_component][pvar] * _porosity[_i];
      dmass += _fluid_density[_i][ph] * _fluid_saturation_nodal[_i][ph] *
               _mass_frac[_i][ph][_fluid_component] * (*_dporosity_dvar)[_i][pvar];
    }
    return _test[_i][_qp] * _decay_rate * dmass;
  }
  return 0.0;
}

template class PorousFlowMassRadioactiveDecayTempl<false>;
template class PorousFlowMassRadioactiveDecayTempl<true>;
