/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMassVolumetricExpansion.h"

template<>
InputParameters validParams<PorousFlowMassVolumetricExpansion>()
{
  InputParameters params = validParams<TimeKernel>();
  params.addParam<unsigned int>("component_index", 0, "The index corresponding to the component for this kernel");
  params.addRequiredCoupledVar("displacements", "TODO - this is only necessary because the tensor-mechanics strain Materials are not derivative materials.  The displacements appropriate for the simulation geometry and coordinate system");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("Component_mass*rate_of_solid_volumetric_expansion");
  return params;
}

PorousFlowMassVolumetricExpansion::PorousFlowMassVolumetricExpansion(const InputParameters & parameters) :
  TimeKernel(parameters),
  _component_index(getParam<unsigned int>("component_index")),
  _dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
  _var_is_porflow_var(!(_dictator_UO.not_porflow_var(_var.number()))),
  _ndisp(coupledComponents("displacements")),
  _disp_var_num(_ndisp),
  _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
  _dporosity_dvar(getMaterialProperty<std::vector<Real> >("dPorousFlow_porosity_nodal_dvar")),
  _dporosity_dgradvar(getMaterialProperty<std::vector<RealGradient> >("dPorousFlow_porosity_nodal_dgradvar")),
  _fluid_density(getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density")),
  _dfluid_density_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_fluid_phase_density_dvar")),
  _fluid_saturation(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation_nodal")),
  _dfluid_saturation_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_nodal_dvar")),
  _mass_frac(getMaterialProperty<std::vector<std::vector<Real> > >("PorousFlow_mass_frac")),
  _dmass_frac_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real> > > >("dPorousFlow_mass_frac_dvar")),
  _strain_rate_qp(getMaterialProperty<Real>("PorousFlow_volumetric_strain_rate_qp")),
  _dstrain_rate_qp_dvar(getMaterialProperty<std::vector<RealGradient> >("dPorousFlow_volumetric_strain_rate_qp_dvar"))
{
  if (_component_index >= _dictator_UO.num_components())
    mooseError("The Dictator proclaims that the number of components in this simulation is " << _dictator_UO.num_components() << " whereas you have used the Kernel PorousFlowComponetMassVolumetricExpansion with component = " << _component_index << ".  The Dictator is watching you");
  for (unsigned i = 0 ; i < _ndisp ; ++i)
    _disp_var_num[i] = coupled("displacements", i);
}

Real
PorousFlowMassVolumetricExpansion::computeQpResidual()
{
  mooseAssert(_component_index < _mass_frac[_i][0].size(), "PorousFlowMassVolumetricExpansion: component_index is given as " << _component_index << " which must be less than the number of fluid components described by the mass-fraction matrix, which is " << _mass_frac[_i][0].size());
  unsigned int num_phases = _fluid_density[_i].size();
  mooseAssert(num_phases == _fluid_saturation[_i].size(), "PorousFlowMassVolumetricExpansion: Size of fluid density = " << num_phases << " size of fluid saturation = " << _fluid_saturation[_i].size() << " but both these must be equal to the number of phases in the system");

  Real mass = 0.;
  for (unsigned ph = 0; ph < num_phases; ++ph)
    mass += _fluid_density[_i][ph]*_fluid_saturation[_i][ph]*_mass_frac[_i][ph][_component_index];

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
PorousFlowMassVolumetricExpansion::computedVolQpJac(unsigned int jvar)
{
  if (_dictator_UO.not_porflow_var(jvar))
    return 0.0;

  const unsigned int pvar = _dictator_UO.porflow_var_num(jvar);

  unsigned int num_phases = _fluid_density[_i].size();
  Real mass = 0.;
  for (unsigned ph = 0; ph < num_phases; ++ph)
    mass += _fluid_density[_i][ph]*_fluid_saturation[_i][ph]*_mass_frac[_i][ph][_component_index];

  Real dvol = _dstrain_rate_qp_dvar[_qp][pvar]*_grad_phi[_j][_qp];
  /*
  for (unsigned i = 0 ; i < _ndisp ; ++i)
    if (jvar == _disp_var_num[i])
      dvol = _grad_phi[_j][_qp](i);
  */

  //return _test[_i][_qp] * mass * _porosity[_i] * dvol/_dt;
  return _test[_i][_qp] * mass * _porosity[_i] * dvol;
}
Real
PorousFlowMassVolumetricExpansion::computedMassQpJac(unsigned int jvar)
{
  if (_dictator_UO.not_porflow_var(jvar))
    return 0.0;

  const unsigned int pvar = _dictator_UO.porflow_var_num(jvar);

  const unsigned int num_phases = _fluid_density[_i].size();
  Real dmass = 0.0;
  for (unsigned ph = 0; ph < num_phases; ++ph)
    dmass += _fluid_density[_i][ph]*_fluid_saturation[_i][ph]*_mass_frac[_i][ph][_component_index]*_dporosity_dgradvar[_i][pvar]*_grad_phi[_j][_i];

  if (_i != _j)
    return _test[_i][_qp]*dmass*_strain_rate_qp[_qp];


  for (unsigned ph = 0; ph < num_phases; ++ph)
  {
    dmass += _dfluid_density_dvar[_i][ph][pvar]*_fluid_saturation[_i][ph]*_mass_frac[_i][ph][_component_index]*_porosity[_i];
    dmass += _fluid_density[_i][ph]*_dfluid_saturation_dvar[_i][ph][pvar]*_mass_frac[_i][ph][_component_index]*_porosity[_i];
    dmass += _fluid_density[_i][ph]*_fluid_saturation[_i][ph]*_dmass_frac_dvar[_i][ph][_component_index][pvar]*_porosity[_i];
    dmass += _fluid_density[_i][ph]*_fluid_saturation[_i][ph]*_mass_frac[_i][ph][_component_index]*_dporosity_dvar[_i][pvar];
  }


  return _test[_i][_qp] * dmass * _strain_rate_qp[_qp];
}
