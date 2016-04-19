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
  params.addParam<unsigned int>("component_index", 0, "The index corresponding to the component for this kernel");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("Component mass derivative wrt time for component given by component_index");
  return params;
}

PorousFlowMassTimeDerivative::PorousFlowMassTimeDerivative(const InputParameters & parameters) :
  TimeKernel(parameters),
  _component_index(getParam<unsigned int>("component_index")),
  _porflow_name_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
  _var_is_porflow_var(!(_porflow_name_UO.not_porflow_var(_var.number()))),
  _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
  _porosity_old(getMaterialPropertyOld<Real>("PorousFlow_porosity_nodal")),
  _dporosity_dvar(getMaterialProperty<std::vector<Real> >("dPorousFlow_porosity_nodal_dvar")),
  _dporosity_dgradvar(getMaterialProperty<std::vector<RealGradient> >("dPorousFlow_porosity_nodal_dgradvar")),
  _fluid_density(getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density")),
  _fluid_density_old(getMaterialPropertyOld<std::vector<Real> >("PorousFlow_fluid_phase_density")),
  _dfluid_density_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_fluid_phase_density_dvar")),
  _fluid_saturation(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation")),
  _fluid_saturation_old(getMaterialPropertyOld<std::vector<Real> >("PorousFlow_saturation")),
  _dfluid_saturation_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_dvar")),
  _mass_frac(getMaterialProperty<std::vector<std::vector<Real> > >("PorousFlow_mass_frac")),
  _mass_frac_old(getMaterialPropertyOld<std::vector<std::vector<Real> > >("PorousFlow_mass_frac")),
  _dmass_frac_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real> > > >("dPorousFlow_mass_frac_dvar"))
{
  if (_component_index >= _porflow_name_UO.num_components())
    mooseError("The Dictator proclaims that the number of components in this simulation is " << _porflow_name_UO.num_components() << " whereas you have used the Kernel PorousFlowComponetMassTimeDerivative with component = " << _component_index << ".  The Dictator does not take such mistakes lightly");
}

/// Note that this kernel lumps the mass terms to the nodes, so that there is no mass at the qp's.
Real
PorousFlowMassTimeDerivative::computeQpResidual()
{
  mooseAssert(_component_index < _mass_frac[_i][0].size(), "PorousFlowMassTimeDerivative: component_index is given as " << _component_index << " which must be less than the number of fluid components described by the mass-fraction matrix, which is " << _mass_frac[_i][0].size());
  unsigned int num_phases = _fluid_density[_i].size();
  mooseAssert(num_phases == _fluid_saturation[_i].size(), "PorousFlowMassTimeDerivative: Size of fluid density = " << num_phases << " size of fluid saturation = " << _fluid_saturation[_i].size() << " but both these must be equal to the number of phases in the system");

  Real mass = 0.;
  Real mass_old = 0.;
  for (unsigned ph = 0; ph < num_phases; ++ph)
  {
    mass += _fluid_density[_i][ph]*_fluid_saturation[_i][ph]*_mass_frac[_i][ph][_component_index];
    mass_old += _fluid_density_old[_i][ph]*_fluid_saturation_old[_i][ph]*_mass_frac_old[_i][ph][_component_index];
   }

  return _test[_i][_qp] * (_porosity[_i]*mass - _porosity_old[_i]*mass_old)/_dt;
}

Real
PorousFlowMassTimeDerivative::computeQpJacobian()
{
  /// If the variable is not a PorousFlow variable (very unusual), the diag Jacobian terms are 0
  if (!(_var_is_porflow_var))
    return 0.0;
  return computeQpJac(_porflow_name_UO.porflow_var_num(_var.number()));
}

Real
PorousFlowMassTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  /// If the variable is not a PorousFlow variable, the OffDiag Jacobian terms are 0
  if (_porflow_name_UO.not_porflow_var(jvar))
    return 0.0;
  return computeQpJac(_porflow_name_UO.porflow_var_num(jvar));
}

Real
PorousFlowMassTimeDerivative::computeQpJac(unsigned int pvar)
{
  /// As mass is lumped to the nodes, only non-zero terms are for _i==_j
  if (_i != _j)
    return 0.0;

  unsigned int num_phases = _fluid_density[_i].size();

  Real dmass = 0.0;
  for (unsigned ph = 0; ph < num_phases; ++ph)
  {
    dmass += _dfluid_density_dvar[_i][ph][pvar]*_fluid_saturation[_i][ph]*_mass_frac[_i][ph][_component_index]*_porosity[_i];
    dmass += _fluid_density[_i][ph]*_dfluid_saturation_dvar[_i][ph][pvar]*_mass_frac[_i][ph][_component_index]*_porosity[_i];
    dmass += _fluid_density[_i][ph]*_fluid_saturation[_i][ph]*_dmass_frac_dvar[_i][ph][_component_index][pvar]*_porosity[_i];
    dmass += _fluid_density[_i][ph]*_fluid_saturation[_i][ph]*_mass_frac[_i][ph][_component_index]*_dporosity_dvar[_i][pvar];
  }
  return _test[_i][_qp]*dmass/_dt;
}
