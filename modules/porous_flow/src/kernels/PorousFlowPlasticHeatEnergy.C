/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPlasticHeatEnergy.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<PorousFlowPlasticHeatEnergy>()
{
  InputParameters params = validParams<PlasticHeatEnergy>();
  params.addRequiredParam<UserObjectName>("PorousFlowDictator", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("Plastic heat energy density source = (1 - porosity) * coeff * stress * plastic_strain_rate");
  return params;
}


PorousFlowPlasticHeatEnergy::PorousFlowPlasticHeatEnergy(const InputParameters & parameters) :
    PlasticHeatEnergy(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
    _dporosity_dvar(getMaterialProperty<std::vector<Real> >("dPorousFlow_porosity_nodal_dvar")),
    _dporosity_dgradvar(getMaterialProperty<std::vector<RealGradient> >("dPorousFlow_porosity_nodal_dgradvar"))
{
}

Real
PorousFlowPlasticHeatEnergy::computeQpResidual()
{
  return (1.0 - _porosity[_i]) * PlasticHeatEnergy::computeQpResidual();
}

Real
PorousFlowPlasticHeatEnergy::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
PorousFlowPlasticHeatEnergy::computeQpOffDiagJacobian(unsigned int jvar)
{
  /// If the variable is not a PorousFlow variable, the Jacobian terms are 0
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;

  const Real res_no_porosity = PlasticHeatEnergy::computeQpResidual();
  const Real jac_no_porosity = PlasticHeatEnergy::computeQpOffDiagJacobian(jvar);

  const unsigned pvar = _dictator.porousFlowVariableNum(jvar);

  Real jac = (1.0 - _porosity[_i]) * jac_no_porosity - _dporosity_dgradvar[_i][pvar] * _grad_phi[_j][_i] * res_no_porosity;
  if (_i != _j)
    return jac;

  return jac - _dporosity_dvar[_i][pvar] * res_no_porosity;
}
