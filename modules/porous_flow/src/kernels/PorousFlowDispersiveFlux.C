//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDispersiveFlux.h"

#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowDispersiveFlux);

InputParameters
PorousFlowDispersiveFlux::validParams()
{
  InputParameters params = Kernel::validParams();
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

PorousFlowDispersiveFlux::PorousFlowDispersiveFlux(const InputParameters & parameters)
  : Kernel(parameters),

    _fluid_density_qp(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _dfluid_density_qp_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_density_qp_dvar")),
    _grad_mass_frac(getMaterialProperty<std::vector<std::vector<RealGradient>>>(
        "PorousFlow_grad_mass_frac_qp")),
    _dmass_frac_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
        "dPorousFlow_mass_frac_qp_dvar")),
    _porosity_qp(getMaterialProperty<Real>("PorousFlow_porosity_qp")),
    _dporosity_qp_dvar(getMaterialProperty<std::vector<Real>>("dPorousFlow_porosity_qp_dvar")),
    _tortuosity(getMaterialProperty<std::vector<Real>>("PorousFlow_tortuosity_qp")),
    _dtortuosity_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_tortuosity_qp_dvar")),
    _diffusion_coeff(
        getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_diffusion_coeff_qp")),
    _ddiffusion_coeff_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
        "dPorousFlow_diffusion_coeff_qp_dvar")),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _num_phases(_dictator.numPhases()),
    _identity_tensor(RankTwoTensor::initIdentity),
    _relative_permeability(
        getMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_qp")),
    _drelative_permeability_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_relative_permeability_qp_dvar")),
    _fluid_viscosity(getMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_qp")),
    _dfluid_viscosity_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_viscosity_qp_dvar")),
    _permeability(getMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _dpermeability_dvar(
        getMaterialProperty<std::vector<RealTensorValue>>("dPorousFlow_permeability_qp_dvar")),
    _dpermeability_dgradvar(getMaterialProperty<std::vector<std::vector<RealTensorValue>>>(
        "dPorousFlow_permeability_qp_dgradvar")),
    _grad_p(getMaterialProperty<std::vector<RealGradient>>("PorousFlow_grad_porepressure_qp")),
    _dgrad_p_dgrad_var(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_grad_porepressure_qp_dgradvar")),
    _dgrad_p_dvar(getMaterialProperty<std::vector<std::vector<RealGradient>>>(
        "dPorousFlow_grad_porepressure_qp_dvar")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _disp_long(getParam<std::vector<Real>>("disp_long")),
    _disp_trans(getParam<std::vector<Real>>("disp_trans")),
    _perm_derivs(_dictator.usePermDerivs())
{
  // Check that sufficient values of the dispersion coefficients have been entered
  if (_disp_long.size() != _num_phases)
    paramError(
        "disp_long",
        "The number of longitudinal dispersion coefficients is not equal to the number of phases");

  if (_disp_trans.size() != _num_phases)
    paramError("disp_trans",
               "The number of transverse dispersion coefficients disp_trans in is not equal to the "
               "number of phases");
}

Real
PorousFlowDispersiveFlux::computeQpResidual()
{
  RealVectorValue flux = 0.0;
  RealVectorValue velocity;
  Real velocity_abs;
  RankTwoTensor v2;
  RankTwoTensor dispersion;
  dispersion.zero();
  Real diffusion;

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    // Diffusive component
    diffusion =
        _porosity_qp[_qp] * _tortuosity[_qp][ph] * _diffusion_coeff[_qp][ph][_fluid_component];

    // Calculate Darcy velocity
    velocity = (_permeability[_qp] * (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity) *
                _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph]);
    velocity_abs = std::sqrt(velocity * velocity);

    if (velocity_abs > 0.0)
    {
      v2 = RankTwoTensor::selfOuterProduct(velocity);

      // Add longitudinal dispersion to diffusive component
      diffusion += _disp_trans[ph] * velocity_abs;
      dispersion = (_disp_long[ph] - _disp_trans[ph]) * v2 / velocity_abs;
    }

    flux += _fluid_density_qp[_qp][ph] * (diffusion * _identity_tensor + dispersion) *
            _grad_mass_frac[_qp][ph][_fluid_component];
  }
  return _grad_test[_i][_qp] * flux;
}

Real
PorousFlowDispersiveFlux::computeQpJacobian()
{
  return computeQpJac(_var.number());
}

Real
PorousFlowDispersiveFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  return computeQpJac(jvar);
}

Real
PorousFlowDispersiveFlux::computeQpJac(unsigned int jvar) const
{
  // If the variable is not a valid PorousFlow variable, set the Jacobian to 0
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
    // Diffusive component
    diffusion =
        _porosity_qp[_qp] * _tortuosity[_qp][ph] * _diffusion_coeff[_qp][ph][_fluid_component];

    // Calculate Darcy velocity
    velocity = (_permeability[_qp] * (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity) *
                _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph]);
    velocity_abs = std::sqrt(velocity * velocity);

    if (velocity_abs > 0.0)
    {
      v2 = RankTwoTensor::selfOuterProduct(velocity);

      // Add longitudinal dispersion to diffusive component
      diffusion += _disp_trans[ph] * velocity_abs;
      dispersion = (_disp_long[ph] - _disp_trans[ph]) * v2 / velocity_abs;
    }

    // Derivative of Darcy velocity
    RealVectorValue dvelocity =
        _permeability[_qp] * (_grad_phi[_j][_qp] * _dgrad_p_dgrad_var[_qp][ph][pvar] -
                              _phi[_j][_qp] * _dfluid_density_qp_dvar[_qp][ph][pvar] * _gravity);
    dvelocity += _permeability[_qp] * (_dgrad_p_dvar[_qp][ph][pvar] * _phi[_j][_qp]);

    if (_perm_derivs)
    {
      dvelocity += _dpermeability_dvar[_qp][pvar] * _phi[_j][_qp] *
                   (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity);

      for (const auto i : make_range(Moose::dim))
        dvelocity += _dpermeability_dgradvar[_qp][i][pvar] * _grad_phi[_j][_qp](i) *
                     (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity);
    }

    dvelocity = dvelocity * _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph] +
                (_permeability[_qp] * (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity)) *
                    (_drelative_permeability_dvar[_qp][ph][pvar] / _fluid_viscosity[_qp][ph] -
                     _relative_permeability[_qp][ph] * _dfluid_viscosity_dvar[_qp][ph][pvar] /
                         std::pow(_fluid_viscosity[_qp][ph], 2)) *
                    _phi[_j][_qp];

    Real dvelocity_abs = 0.0;
    if (velocity_abs > 0.0)
      dvelocity_abs = velocity * dvelocity / velocity_abs;

    // Derivative of diffusion term (note: dispersivity is assumed constant)
    Real ddiffusion = _phi[_j][_qp] * _dporosity_qp_dvar[_qp][pvar] * _tortuosity[_qp][ph] *
                      _diffusion_coeff[_qp][ph][_fluid_component];
    ddiffusion += _phi[_j][_qp] * _porosity_qp[_qp] * _dtortuosity_dvar[_qp][ph][pvar] *
                  _diffusion_coeff[_qp][ph][_fluid_component];
    ddiffusion += _phi[_j][_qp] * _porosity_qp[_qp] * _tortuosity[_qp][ph] *
                  _ddiffusion_coeff_dvar[_qp][ph][_fluid_component][pvar];
    ddiffusion += _disp_trans[ph] * dvelocity_abs;

    // Derivative of dispersion term (note: dispersivity is assumed constant)
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

    dflux += _phi[_j][_qp] * _dfluid_density_qp_dvar[_qp][ph][pvar] *
             (diffusion * _identity_tensor + dispersion) *
             _grad_mass_frac[_qp][ph][_fluid_component];
    dflux += _fluid_density_qp[_qp][ph] * (ddiffusion * _identity_tensor + ddispersion) *
             _grad_mass_frac[_qp][ph][_fluid_component];

    // NOTE: Here we assume that d(grad_mass_frac)/d(var) = d(mass_frac)/d(var) * grad_phi
    //       This is true for most PorousFlow scenarios, but not for chemical reactions
    //       where mass_frac is a nonlinear function of the primary MOOSE Variables
    dflux += _fluid_density_qp[_qp][ph] * (diffusion * _identity_tensor + dispersion) *
             _dmass_frac_dvar[_qp][ph][_fluid_component][pvar] * _grad_phi[_j][_qp];
  }

  return _grad_test[_i][_qp] * dflux;
}
