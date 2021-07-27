//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMassVolumetricExpansion.h"

#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowMassVolumetricExpansion);

InputParameters
PorousFlowMassVolumetricExpansion::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params.addParam<bool>("strain_at_nearest_qp",
                        false,
                        "When calculating nodal porosity that depends on strain, use the strain at "
                        "the nearest quadpoint.  This adds a small extra computational burden, and "
                        "is not necessary for simulations involving only linear lagrange elements. "
                        " If you set this to true, you will also want to set the same parameter to "
                        "true for related Kernels and Materials");
  params.addParam<bool>(
      "multiply_by_density",
      true,
      "If true, then this Kernel represents component_mass*rate_of_solid_volumetric_expansion.  If "
      "flase, then this Kernel represents component_volume*rate_of_solid_volumetric_expansion "
      "(care must then be taken when using other PorousFlow objects, such as the "
      "PorousFlowFluidMass postprocessor).");
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index corresponding to the component for this kernel");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.set<bool>("use_displaced_mesh") = false;
  params.suppressParameter<bool>("use_displaced_mesh");
  params.addClassDescription("Component_mass*rate_of_solid_volumetric_expansion.  This Kernel "
                             "lumps the component mass to the nodes.");
  return params;
}

PorousFlowMassVolumetricExpansion::PorousFlowMassVolumetricExpansion(
    const InputParameters & parameters)
  : TimeKernel(parameters),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _var_is_porflow_var(!_dictator.notPorousFlowVariable(_var.number())),
    _num_phases(_dictator.numPhases()),
    _strain_at_nearest_qp(getParam<bool>("strain_at_nearest_qp")),
    _multiply_by_density(getParam<bool>("multiply_by_density")),
    _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
    _dporosity_dvar(getMaterialProperty<std::vector<Real>>("dPorousFlow_porosity_nodal_dvar")),
    _dporosity_dgradvar(
        getMaterialProperty<std::vector<RealGradient>>("dPorousFlow_porosity_nodal_dgradvar")),
    _nearest_qp(_strain_at_nearest_qp
                    ? &getMaterialProperty<unsigned int>("PorousFlow_nearestqp_nodal")
                    : nullptr),
    _fluid_density(_multiply_by_density ? &getMaterialProperty<std::vector<Real>>(
                                              "PorousFlow_fluid_phase_density_nodal")
                                        : nullptr),
    _dfluid_density_dvar(_multiply_by_density
                             ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                   "dPorousFlow_fluid_phase_density_nodal_dvar")
                             : nullptr),
    _fluid_saturation(getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_nodal")),
    _dfluid_saturation_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_saturation_nodal_dvar")),
    _mass_frac(getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")),
    _dmass_frac_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
        "dPorousFlow_mass_frac_nodal_dvar")),
    _strain_rate_qp(getMaterialProperty<Real>("PorousFlow_volumetric_strain_rate_qp")),
    _dstrain_rate_qp_dvar(getMaterialProperty<std::vector<RealGradient>>(
        "dPorousFlow_volumetric_strain_rate_qp_dvar"))
{
  if (_fluid_component >= _dictator.numComponents())
    mooseError("The Dictator proclaims that the number of components in this simulation is ",
               _dictator.numComponents(),
               " whereas you have used the Kernel PorousFlowComponetMassVolumetricExpansion with "
               "component = ",
               _fluid_component,
               ".  The Dictator is watching you");
}

Real
PorousFlowMassVolumetricExpansion::computeQpResidual()
{
  Real mass = 0.0;
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    const Real dens = (_multiply_by_density ? (*_fluid_density)[_i][ph] : 1.0);
    mass += dens * _fluid_saturation[_i][ph] * _mass_frac[_i][ph][_fluid_component];
  }

  return _test[_i][_qp] * mass * _porosity[_i] * _strain_rate_qp[_qp];
}

Real
PorousFlowMassVolumetricExpansion::computeQpJacobian()
{
  return computedMassQpJac(_var.number()) + computedVolQpJac(_var.number());
}

Real
PorousFlowMassVolumetricExpansion::computeQpOffDiagJacobian(unsigned int jvar)
{
  return computedMassQpJac(jvar) + computedVolQpJac(jvar);
}

Real
PorousFlowMassVolumetricExpansion::computedVolQpJac(unsigned int jvar) const
{
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;

  const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);

  Real mass = 0.0;
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    const Real dens = (_multiply_by_density ? (*_fluid_density)[_i][ph] : 1.0);
    mass += dens * _fluid_saturation[_i][ph] * _mass_frac[_i][ph][_fluid_component];
  }

  Real dvol = _dstrain_rate_qp_dvar[_qp][pvar] * _grad_phi[_j][_qp];

  return _test[_i][_qp] * mass * _porosity[_i] * dvol;
}
Real
PorousFlowMassVolumetricExpansion::computedMassQpJac(unsigned int jvar) const
{
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;

  const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);
  const unsigned nearest_qp = (_strain_at_nearest_qp ? (*_nearest_qp)[_i] : _i);

  Real dmass = 0.0;
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    const Real dens = (_multiply_by_density ? (*_fluid_density)[_i][ph] : 1.0);
    dmass += dens * _fluid_saturation[_i][ph] * _mass_frac[_i][ph][_fluid_component] *
             _dporosity_dgradvar[_i][pvar] * _grad_phi[_j][nearest_qp];
  }

  if (_i != _j)
    return _test[_i][_qp] * dmass * _strain_rate_qp[_qp];

  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    if (_multiply_by_density)
      dmass += (*_dfluid_density_dvar)[_i][ph][pvar] * _fluid_saturation[_i][ph] *
               _mass_frac[_i][ph][_fluid_component] * _porosity[_i];
    const Real dens = (_multiply_by_density ? (*_fluid_density)[_i][ph] : 1.0);
    dmass += dens * _dfluid_saturation_dvar[_i][ph][pvar] * _mass_frac[_i][ph][_fluid_component] *
             _porosity[_i];
    dmass += dens * _fluid_saturation[_i][ph] * _dmass_frac_dvar[_i][ph][_fluid_component][pvar] *
             _porosity[_i];
    dmass += dens * _fluid_saturation[_i][ph] * _mass_frac[_i][ph][_fluid_component] *
             _dporosity_dvar[_i][pvar];
  }

  return _test[_i][_qp] * dmass * _strain_rate_qp[_qp];
}
