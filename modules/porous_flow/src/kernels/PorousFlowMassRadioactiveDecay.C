//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

InputParameters
PorousFlowMassRadioactiveDecay::validParams()
{
  InputParameters params = TimeKernel::validParams();
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

PorousFlowMassRadioactiveDecay::PorousFlowMassRadioactiveDecay(const InputParameters & parameters)
  : TimeKernel(parameters),
    _decay_rate(getParam<Real>("decay_rate")),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _var_is_porflow_var(_dictator.isPorousFlowVariable(_var.number())),
    _num_phases(_dictator.numPhases()),
    _strain_at_nearest_qp(getParam<bool>("strain_at_nearest_qp")),
    _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
    _dporosity_dvar(getMaterialProperty<std::vector<Real>>("dPorousFlow_porosity_nodal_dvar")),
    _dporosity_dgradvar(
        getMaterialProperty<std::vector<RealGradient>>("dPorousFlow_porosity_nodal_dgradvar")),
    _nearest_qp(_strain_at_nearest_qp
                    ? &getMaterialProperty<unsigned int>("PorousFlow_nearestqp_nodal")
                    : nullptr),
    _fluid_density(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_nodal")),
    _dfluid_density_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_density_nodal_dvar")),
    _fluid_saturation_nodal(getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_nodal")),
    _dfluid_saturation_nodal_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_saturation_nodal_dvar")),
    _mass_frac(getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")),
    _dmass_frac_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
        "dPorousFlow_mass_frac_nodal_dvar"))
{
  if (_fluid_component >= _dictator.numComponents())
    paramError(
        "fluid_component",
        "The Dictator proclaims that the maximum fluid component index in this simulation is ",
        _dictator.numComponents() - 1,
        " whereas you have used ",
        _fluid_component,
        ". Remember that indexing starts at 0. The Dictator does not take such mistakes lightly.");
}

Real
PorousFlowMassRadioactiveDecay::computeQpResidual()
{
  Real mass = 0.0;
  for (unsigned ph = 0; ph < _num_phases; ++ph)
    mass += _fluid_density[_i][ph] * _fluid_saturation_nodal[_i][ph] *
            _mass_frac[_i][ph][_fluid_component];

  return _test[_i][_qp] * _decay_rate * _porosity[_i] * mass;
}

Real
PorousFlowMassRadioactiveDecay::computeQpJacobian()
{
  // If the variable is not a PorousFlow variable (very unusual), the diag Jacobian terms are 0
  if (!_var_is_porflow_var)
    return 0.0;
  return computeQpJac(_dictator.porousFlowVariableNum(_var.number()));
}

Real
PorousFlowMassRadioactiveDecay::computeQpOffDiagJacobian(unsigned int jvar)
{
  // If the variable is not a PorousFlow variable, the OffDiag Jacobian terms are 0
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;
  return computeQpJac(_dictator.porousFlowVariableNum(jvar));
}

Real
PorousFlowMassRadioactiveDecay::computeQpJac(unsigned int pvar)
{
  const unsigned nearest_qp = (_strain_at_nearest_qp ? (*_nearest_qp)[_i] : _i);

  // porosity is dependent on variables that are lumped to the nodes,
  // but it can depend on the gradient
  // of variables, which are NOT lumped to the nodes, hence:
  Real dmass = 0.0;
  for (unsigned ph = 0; ph < _num_phases; ++ph)
    dmass += _fluid_density[_i][ph] * _fluid_saturation_nodal[_i][ph] *
             _mass_frac[_i][ph][_fluid_component] * _dporosity_dgradvar[_i][pvar] *
             _grad_phi[_j][nearest_qp];

  if (_i != _j)
    return _test[_i][_qp] * _decay_rate * dmass;

  // As the fluid mass is lumped to the nodes, only non-zero terms are for _i==_j
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    dmass += _dfluid_density_dvar[_i][ph][pvar] * _fluid_saturation_nodal[_i][ph] *
             _mass_frac[_i][ph][_fluid_component] * _porosity[_i];
    dmass += _fluid_density[_i][ph] * _dfluid_saturation_nodal_dvar[_i][ph][pvar] *
             _mass_frac[_i][ph][_fluid_component] * _porosity[_i];
    dmass += _fluid_density[_i][ph] * _fluid_saturation_nodal[_i][ph] *
             _dmass_frac_dvar[_i][ph][_fluid_component][pvar] * _porosity[_i];
    dmass += _fluid_density[_i][ph] * _fluid_saturation_nodal[_i][ph] *
             _mass_frac[_i][ph][_fluid_component] * _dporosity_dvar[_i][pvar];
  }
  return _test[_i][_qp] * _decay_rate * dmass;
}
