//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDispersiveFlux.h"

#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowDispersiveFlux);
registerMooseObject("PorousFlowApp", ADPorousFlowDispersiveFlux);

template <bool is_ad>
InputParameters
PorousFlowDispersiveFluxTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index corresponding to the fluid component for this kernel");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addRequiredParam<std::vector<Real>>(
      "disp_long", "Vector of longitudinal dispersion coefficients for each phase");
  params.addRequiredParam<std::vector<Real>>(
      "disp_trans", "Vector of transverse dispersion coefficients for each phase");
  params.addRequiredParam<RealVectorValue>("gravity",
                                           "Gravitational acceleration vector downwards (m/s^2)");
  params.addClassDescription(
      "Dispersive and diffusive flux of the component given by fluid_component in all phases");
  return params;
}

template <bool is_ad>
PorousFlowDispersiveFluxTempl<is_ad>::PorousFlowDispersiveFluxTempl(
    const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _fluid_density_qp(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_fluid_phase_density_qp")),
    _dfluid_density_qp_dvar(
        is_ad ? nullptr
              : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                    "dPorousFlow_fluid_phase_density_qp_dvar")),
    _grad_mass_frac(
        this->template getGenericMaterialProperty<std::vector<std::vector<RealGradient>>, is_ad>(
            "PorousFlow_grad_mass_frac_qp")),
    _dmass_frac_dvar(
        is_ad ? nullptr
              : &this->template getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
                    "dPorousFlow_mass_frac_qp_dvar")),
    _porosity_qp(this->template getGenericMaterialProperty<Real, is_ad>("PorousFlow_porosity_qp")),
    _dporosity_qp_dvar(is_ad ? nullptr
                             : &this->template getMaterialProperty<std::vector<Real>>(
                                   "dPorousFlow_porosity_qp_dvar")),
    _tortuosity(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_tortuosity_qp")),
    _dtortuosity_dvar(is_ad ? nullptr
                            : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                                  "dPorousFlow_tortuosity_qp_dvar")),
    _diffusion_coeff(this->template getMaterialProperty<std::vector<std::vector<Real>>>(
        "PorousFlow_diffusion_coeff_qp")),
    _ddiffusion_coeff_dvar(
        is_ad ? nullptr
              : &this->template getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
                    "dPorousFlow_diffusion_coeff_qp_dvar")),
    _dictator(this->template getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _fluid_component(this->template getParam<unsigned int>("fluid_component")),
    _num_phases(_dictator.numPhases()),
    _identity_tensor(GenericRankTwoTensor<is_ad>::initIdentity),
    _relative_permeability(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_relative_permeability_qp")),
    _drelative_permeability_dvar(
        is_ad ? nullptr
              : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                    "dPorousFlow_relative_permeability_qp_dvar")),
    _fluid_viscosity(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_viscosity_qp")),
    _dfluid_viscosity_dvar(
        is_ad ? nullptr
              : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                    "dPorousFlow_viscosity_qp_dvar")),
    _permeability(this->template getGenericMaterialProperty<RealTensorValue, is_ad>(
        "PorousFlow_permeability_qp")),
    _dpermeability_dvar(is_ad ? nullptr
                              : &this->template getMaterialProperty<std::vector<RealTensorValue>>(
                                    "dPorousFlow_permeability_qp_dvar")),
    _dpermeability_dgradvar(
        is_ad ? nullptr
              : &this->template getMaterialProperty<std::vector<std::vector<RealTensorValue>>>(
                    "dPorousFlow_permeability_qp_dgradvar")),
    _grad_p(this->template getGenericMaterialProperty<std::vector<RealGradient>, is_ad>(
        "PorousFlow_grad_porepressure_qp")),
    _dgrad_p_dgrad_var(is_ad ? nullptr
                             : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                                   "dPorousFlow_grad_porepressure_qp_dgradvar")),
    _dgrad_p_dvar(is_ad
                      ? nullptr
                      : &this->template getMaterialProperty<std::vector<std::vector<RealGradient>>>(
                            "dPorousFlow_grad_porepressure_qp_dvar")),
    _gravity(this->template getParam<RealVectorValue>("gravity")),
    _disp_long(this->template getParam<std::vector<Real>>("disp_long")),
    _disp_trans(this->template getParam<std::vector<Real>>("disp_trans")),
    _perm_derivs(_dictator.usePermDerivs())
{
  if (_fluid_component >= _dictator.numComponents())
    this->paramError(
        "fluid_component",
        "The Dictator proclaims that the maximum fluid component index in this simulation is ",
        _dictator.numComponents() - 1,
        " whereas you have used ",
        _fluid_component,
        ". Remember that indexing starts at 0. The Dictator does not take such mistakes lightly.");

  if (_disp_long.size() != _num_phases)
    this->paramError(
        "disp_long",
        "The number of longitudinal dispersion coefficients is not equal to the number of phases");

  if (_disp_trans.size() != _num_phases)
    this->paramError("disp_trans",
                     "The number of transverse dispersion coefficients disp_trans is not equal to "
                     "the number of phases");
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowDispersiveFluxTempl<is_ad>::computeQpResidual()
{
  GenericRealVectorValue<is_ad> flux;
  GenericRealVectorValue<is_ad> velocity;
  GenericReal<is_ad> velocity_abs;
  GenericRankTwoTensor<is_ad> v2;
  GenericRankTwoTensor<is_ad> dispersion;
  dispersion.zero();
  GenericReal<is_ad> diffusion;

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    diffusion =
        _porosity_qp[_qp] * _tortuosity[_qp][ph] * _diffusion_coeff[_qp][ph][_fluid_component];

    velocity = _permeability[_qp] * (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity) *
               _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph];
    velocity_abs = velocity.norm();

    if (MetaPhysicL::raw_value(velocity_abs) > 0.0)
    {
      v2 = GenericRankTwoTensor<is_ad>::selfOuterProduct(velocity);
      diffusion += GenericReal<is_ad>(_disp_trans[ph]) * velocity_abs;
      dispersion = GenericReal<is_ad>(_disp_long[ph] - _disp_trans[ph]) * v2 / velocity_abs;
    }

    flux += _fluid_density_qp[_qp][ph] * (diffusion * _identity_tensor + dispersion) *
            _grad_mass_frac[_qp][ph][_fluid_component];
  }
  return _grad_test[_i][_qp] * flux;
}

template <bool is_ad>
Real
PorousFlowDispersiveFluxTempl<is_ad>::computeQpJacobian()
{
  return computeQpJac(_var.number());
}

template <bool is_ad>
Real
PorousFlowDispersiveFluxTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  return computeQpJac(jvar);
}

template <bool is_ad>
Real
PorousFlowDispersiveFluxTempl<is_ad>::computeQpJac(unsigned int jvar) const
{
  if constexpr (!is_ad)
  {
    if (_dictator.notPorousFlowVariable(jvar))
      return 0.0;

    const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);

    RealVectorValue velocity;
    Real velocity_abs;
    RankTwoTensor v2;
    RankTwoTensor dispersion;
    dispersion.zero();
    Real diffusion;
    RealVectorValue dflux = 0.0;

    for (unsigned int ph = 0; ph < _num_phases; ++ph)
    {
      diffusion =
          _porosity_qp[_qp] * _tortuosity[_qp][ph] * _diffusion_coeff[_qp][ph][_fluid_component];

      velocity = _permeability[_qp] * (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity) *
                 _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph];
      velocity_abs = MetaPhysicL::raw_value(velocity).norm();

      if (velocity_abs > 0.0)
      {
        v2 = RankTwoTensor::selfOuterProduct(velocity);
        diffusion += _disp_trans[ph] * velocity_abs;
        dispersion = (_disp_long[ph] - _disp_trans[ph]) * v2 / velocity_abs;
      }

      RealVectorValue dvelocity =
          _permeability[_qp] *
          (_grad_phi[_j][_qp] * (*_dgrad_p_dgrad_var)[_qp][ph][pvar] -
           _phi[_j][_qp] * (*_dfluid_density_qp_dvar)[_qp][ph][pvar] * _gravity);
      dvelocity += _permeability[_qp] * ((*_dgrad_p_dvar)[_qp][ph][pvar] * _phi[_j][_qp]);

      if (_perm_derivs)
      {
        dvelocity += (*_dpermeability_dvar)[_qp][pvar] * _phi[_j][_qp] *
                     (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity);

        for (const auto i : make_range(Moose::dim))
          dvelocity += (*_dpermeability_dgradvar)[_qp][i][pvar] * _grad_phi[_j][_qp](i) *
                       (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity);
      }

      dvelocity =
          dvelocity * _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph] +
          (_permeability[_qp] * (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity)) *
              ((*_drelative_permeability_dvar)[_qp][ph][pvar] / _fluid_viscosity[_qp][ph] -
               _relative_permeability[_qp][ph] * (*_dfluid_viscosity_dvar)[_qp][ph][pvar] /
                   std::pow(_fluid_viscosity[_qp][ph], 2)) *
              _phi[_j][_qp];

      Real dvelocity_abs = 0.0;
      if (velocity_abs > 0.0)
        dvelocity_abs = velocity * dvelocity / velocity_abs;

      Real ddiffusion = _phi[_j][_qp] * (*_dporosity_qp_dvar)[_qp][pvar] * _tortuosity[_qp][ph] *
                        _diffusion_coeff[_qp][ph][_fluid_component];
      ddiffusion += _phi[_j][_qp] * _porosity_qp[_qp] * (*_dtortuosity_dvar)[_qp][ph][pvar] *
                    _diffusion_coeff[_qp][ph][_fluid_component];
      ddiffusion += _phi[_j][_qp] * _porosity_qp[_qp] * _tortuosity[_qp][ph] *
                    (*_ddiffusion_coeff_dvar)[_qp][ph][_fluid_component][pvar];
      ddiffusion += _disp_trans[ph] * dvelocity_abs;

      RankTwoTensor ddispersion;
      ddispersion.zero();
      if (velocity_abs > 0.0)
      {
        RankTwoTensor dv2a, dv2b;
        dv2a = RankTwoTensor::outerProduct(velocity, dvelocity);
        dv2b = RankTwoTensor::outerProduct(dvelocity, velocity);
        ddispersion = (_disp_long[ph] - _disp_trans[ph]) * (dv2a + dv2b) / velocity_abs;
        ddispersion -=
            (_disp_long[ph] - _disp_trans[ph]) * v2 * dvelocity_abs / velocity_abs / velocity_abs;
      }

      dflux += _phi[_j][_qp] * (*_dfluid_density_qp_dvar)[_qp][ph][pvar] *
               (diffusion * RankTwoTensor(RankTwoTensor::initIdentity) + dispersion) *
               _grad_mass_frac[_qp][ph][_fluid_component];
      dflux += _fluid_density_qp[_qp][ph] *
               (ddiffusion * RankTwoTensor(RankTwoTensor::initIdentity) + ddispersion) *
               _grad_mass_frac[_qp][ph][_fluid_component];

      // NOTE: Here we assume that d(grad_mass_frac)/d(var) = d(mass_frac)/d(var) * grad_phi
      //       This is true for most PorousFlow scenarios, but not for chemical reactions
      //       where mass_frac is a nonlinear function of the primary MOOSE Variables
      dflux += _fluid_density_qp[_qp][ph] *
               (diffusion * RankTwoTensor(RankTwoTensor::initIdentity) + dispersion) *
               (*_dmass_frac_dvar)[_qp][ph][_fluid_component][pvar] * _grad_phi[_j][_qp];
    }

    return _grad_test[_i][_qp] * dflux;
  }
  else
    libmesh_ignore(jvar);
  return 0.0;
}

template class PorousFlowDispersiveFluxTempl<false>;
template class PorousFlowDispersiveFluxTempl<true>;
