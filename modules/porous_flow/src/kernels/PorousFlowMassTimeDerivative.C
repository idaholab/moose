/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMassTimeDerivative.h"

template<>
InputParameters validParams<PorousFlowMassTimeDerivative>()
{
  InputParameters params = validParams<TimeKernel>();
  params.addParam<unsigned int>("fluid_component", 0, "The index corresponding to the component for this kernel");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("Component mass derivative wrt time for component given by fluid_component");
  return params;
}

PorousFlowMassTimeDerivative::PorousFlowMassTimeDerivative(const InputParameters & parameters) :
    TimeKernel(parameters),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _var_is_porflow_var(_dictator.isPorousFlowVariable(_var.number())),
    _num_phases(_dictator.numPhases()),
    _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
    _porosity_old(getMaterialPropertyOld<Real>("PorousFlow_porosity_nodal")),
    _dporosity_dvar(getMaterialProperty<std::vector<Real> >("dPorousFlow_porosity_nodal_dvar")),
    _dporosity_dgradvar(getMaterialProperty<std::vector<RealGradient> >("dPorousFlow_porosity_nodal_dgradvar")),
    _fluid_density(getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density")),
    _fluid_density_old(getMaterialPropertyOld<std::vector<Real> >("PorousFlow_fluid_phase_density")),
    _dfluid_density_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_fluid_phase_density_dvar")),
    _fluid_saturation_nodal(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation_nodal")),
    _fluid_saturation_nodal_old(getMaterialPropertyOld<std::vector<Real> >("PorousFlow_saturation_nodal")),
    _dfluid_saturation_nodal_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_nodal_dvar")),
    _mass_frac(getMaterialProperty<std::vector<std::vector<Real> > >("PorousFlow_mass_frac")),
    _mass_frac_old(getMaterialPropertyOld<std::vector<std::vector<Real> > >("PorousFlow_mass_frac")),
    _dmass_frac_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real> > > >("dPorousFlow_mass_frac_dvar"))
{
  if (_fluid_component >= _dictator.numComponents())
    mooseError("The Dictator proclaims that the number of components in this simulation is " << _dictator.numComponents() << " whereas you have used the Kernel PorousFlowComponetMassTimeDerivative with component = " << _fluid_component << ".  The Dictator does not take such mistakes lightly");
}

Real
PorousFlowMassTimeDerivative::computeQpResidual()
{
  Real mass = 0.0;
  Real mass_old = 0.0;
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    mass += _fluid_density[_i][ph] * _fluid_saturation_nodal[_i][ph] * _mass_frac[_i][ph][_fluid_component];
    mass_old += _fluid_density_old[_i][ph] * _fluid_saturation_nodal_old[_i][ph] * _mass_frac_old[_i][ph][_fluid_component];
   }

  return _test[_i][_qp] * (_porosity[_i] * mass - _porosity_old[_i] * mass_old) / _dt;
}

Real
PorousFlowMassTimeDerivative::computeQpJacobian()
{
  /// If the variable is not a PorousFlow variable (very unusual), the diag Jacobian terms are 0
  if (!_var_is_porflow_var)
    return 0.0;
  return computeQpJac(_dictator.porousFlowVariableNum(_var.number()));
}

Real
PorousFlowMassTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  /// If the variable is not a PorousFlow variable, the OffDiag Jacobian terms are 0
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;
  return computeQpJac(_dictator.porousFlowVariableNum(jvar));
}

Real
PorousFlowMassTimeDerivative::computeQpJac(unsigned int pvar)
{
  // porosity is dependent on variables that are lumped to the nodes,
  // but it can depend on the gradient
  // of variables, which are NOT lumped to the nodes, hence:
  Real dmass = 0.0;
  for (unsigned ph = 0; ph < _num_phases; ++ph)
    dmass += _fluid_density[_i][ph] * _fluid_saturation_nodal[_i][ph] * _mass_frac[_i][ph][_fluid_component] * _dporosity_dgradvar[_i][pvar] * _grad_phi[_j][_i];

  if (_i != _j)
    return _test[_i][_qp] * dmass/_dt;

  /// As the fluid mass is lumped to the nodes, only non-zero terms are for _i==_j
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    dmass += _dfluid_density_dvar[_i][ph][pvar] * _fluid_saturation_nodal[_i][ph] * _mass_frac[_i][ph][_fluid_component] * _porosity[_i];
    dmass += _fluid_density[_i][ph] * _dfluid_saturation_nodal_dvar[_i][ph][pvar] * _mass_frac[_i][ph][_fluid_component] * _porosity[_i];
    dmass += _fluid_density[_i][ph] * _fluid_saturation_nodal[_i][ph] * _dmass_frac_dvar[_i][ph][_fluid_component][pvar] * _porosity[_i];
    dmass += _fluid_density[_i][ph] * _fluid_saturation_nodal[_i][ph] * _mass_frac[_i][ph][_fluid_component] * _dporosity_dvar[_i][pvar];
  }
  return _test[_i][_qp] * dmass / _dt;
}
