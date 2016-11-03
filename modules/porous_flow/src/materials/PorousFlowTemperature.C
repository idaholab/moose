/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowTemperature.h"

template<>
InputParameters validParams<PorousFlowTemperature>()
{
  InputParameters params = validParams<Material>();
  params.addCoupledVar("temperature", 20.0, "Fluid temperature variable");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator", "The UserObject that holds the list of Porous-Flow variable names");
  params.addClassDescription("Material to provide temperature and derivatives of it with respect to the PorousFlow variables");
  return params;
}

PorousFlowTemperature::PorousFlowTemperature(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_pf_vars(_dictator.numVariables()),
    _temperature_nodal_var(coupledNodalValue("temperature")),
    _temperature_qp_var(coupledValue("temperature")),
    _grad_temperature(coupledGradient("temperature")),
    _temperature_is_PF(_dictator.isPorousFlowVariable(coupled("temperature"))),
    _t_var_num(_temperature_is_PF ? _dictator.porousFlowVariableNum(coupled("temperature")) : 0),

    _node_number(getMaterialProperty<unsigned int>("PorousFlow_node_number")),

    _temperature_nodal(declareProperty<Real>("PorousFlow_temperature_nodal")),
    _temperature_nodal_old(declarePropertyOld<Real>("PorousFlow_temperature_nodal")),
    _temperature_qp(declareProperty<Real>("PorousFlow_temperature_qp")),
    _gradt_qp(declareProperty<RealGradient>("PorousFlow_grad_temperature_qp")),
    _dtemperature_nodal_dvar(declareProperty<std::vector<Real> >("dPorousFlow_temperature_nodal_dvar")),
    _dtemperature_qp_dvar(declareProperty<std::vector<Real> >("dPorousFlow_temperature_qp_dvar")),
    _dgradt_qp_dgradv(declareProperty<std::vector<Real> >("dPorousFlow_grad_temperature_qp_dgradvar")),
    _dgradt_qp_dv(declareProperty<std::vector<RealGradient> >("dPorousFlow_grad_temperature_qp_dvar"))
{
}

void
PorousFlowTemperature::initQpStatefulProperties()
{
  _temperature_nodal[_qp] = _temperature_nodal_var[_node_number[_qp]];
}

void
PorousFlowTemperature::computeQpProperties()
{
  _temperature_nodal[_qp] = _temperature_nodal_var[_node_number[_qp]];
  _temperature_qp[_qp] = _temperature_qp_var[_qp];
  _gradt_qp[_qp] = _grad_temperature[_qp];

  // Prepare the derivative matrices with zeroes
  _dtemperature_nodal_dvar[_qp].assign(_num_pf_vars, 0.0);
  _dtemperature_qp_dvar[_qp].assign(_num_pf_vars, 0.0);
  _dgradt_qp_dgradv[_qp].assign(_num_pf_vars, 0.0);
  _dgradt_qp_dv[_qp].assign(_num_pf_vars, RealGradient());

  if (_temperature_is_PF)
  {
    // _temperature is a PorousFlow variable
    _dtemperature_nodal_dvar[_qp][_t_var_num] = 1.0;
    _dtemperature_qp_dvar[_qp][_t_var_num] = 1.0;
    _dgradt_qp_dgradv[_qp][_t_var_num] = 1.0;
  }
}
