/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowSink.h"
#include "libmesh/quadrature.h"
#include <iostream>


template<>
InputParameters validParams<PorousFlowSink>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<UserObjectName>("PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addRequiredParam<unsigned int>("fluid_phase", "The index corresponding to the fluid phase for this BC");
  params.addParam<unsigned int>("mass_fraction_component", "The index corresponding to a fluid component.  If supplied, the flux will be multiplied by the nodal mass fraction for the component");
  params.addParam<bool>("use_mobility", false, "If true, then fluxes are multiplied by (density*permeability_nn/viscosity), where the '_nn' indicates the component normal to the boundary.  In this case bare_flux is measured in Pa.s^-1.  This can be used in conjunction with use_relperm.");
  params.addParam<bool>("use_relperm", false, "If true, then fluxes are multiplied by relative permeability.  This can be used in conjunction with use_mobility");
  params.addParam<FunctionName>("flux_function", 1.0, "The flux.  The flux is OUT of the medium: hence positive values of this function means this BC will act as a SINK, while negative values indicate this flux will be a SOURCE.  The functional form is useful for spatially or temporally varying sinks. If use_mobility=false then this function is measured in kg.m^-2.s^-1.  If use_mobility=true then it is measured in Pa.s^-1.");
  return params;
}

PorousFlowSink::PorousFlowSink(const InputParameters & parameters) :
    IntegratedBC(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _ph(getParam<unsigned int>("fluid_phase")),
    _use_mass_fraction(isParamValid("mass_fraction_component")),
    _sp(_use_mass_fraction ? getParam<unsigned int>("mass_fraction_component") : 0),
    _use_mobility(getParam<bool>("use_mobility")),
    _use_relperm(getParam<bool>("use_relperm")),
    _m_func(getFunction("flux_function")),
    _permeability(getMaterialProperty<RealTensorValue>("PorousFlow_permeability")),
    _dpermeability_dvar(getMaterialProperty<std::vector<RealTensorValue> >("dPorousFlow_permeability_dvar")),
    _fluid_density_node(getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density")),
    _dfluid_density_node_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_fluid_phase_density_dvar")),
    _fluid_viscosity(getMaterialProperty<std::vector<Real> >("PorousFlow_viscosity")),
    _dfluid_viscosity_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_viscosity_dvar")),
    _relative_permeability(getMaterialProperty<std::vector<Real> >("PorousFlow_relative_permeability")),
    _drelative_permeability_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_relative_permeability_dvar")),
    _mass_fractions(getMaterialProperty<std::vector<std::vector<Real> > >("PorousFlow_mass_frac")),
    _dmass_fractions_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real> > > >("dPorousFlow_mass_frac_dvar")),
    _node_number(getMaterialProperty<unsigned int>("PorousFlow_node_number"))
{
  if (_ph >= _dictator.numPhases())
    mooseError("PorousFlowSink: The Dictator declares that the number of fluid phases is " << _dictator.numPhases() << ", but you have set the fluid_phase to " << _ph << ".  You must try harder.");
  if (_use_mass_fraction && _sp >= _dictator.numComponents())
    mooseError("PorousFlowSink: The Dictator declares that the number of fluid components is " << _dictator.numComponents() << ", but you have set the mass_fraction_component to " << _sp << ".  Please be assured that the Dictator has noted your error.");
}

void
PorousFlowSink::computeResidual()
{
  _qp_map.assign(_test.size(), -1);
  for (unsigned qp = 0; qp < _qrule->n_points(); qp++)
    _qp_map[_node_number[qp]] = qp;
  IntegratedBC::computeResidual();
}

Real
PorousFlowSink::computeQpResidual()
{
  if (_qp_map[_i] == -1) // the PorousFlowMaterials no not store nodal info for this node (it isn't on the boundary)
    return 0.0;

  const int qp_for_this_node = _qp_map[_i];
  Real flux = _test[_i][_qp] * multiplier();
  if (_use_mobility)
  {
    const Real k = (_permeability[_qp] * _normals[_qp]) * _normals[_qp]; // do not upwind permeability
    flux *= _fluid_density_node[qp_for_this_node][_ph] * k / _fluid_viscosity[qp_for_this_node][_ph];
  }
  if (_use_relperm)
    flux *= _relative_permeability[qp_for_this_node][_ph];
  if (_use_mass_fraction)
    flux *= _mass_fractions[qp_for_this_node][_ph][_sp];
  return flux;
}

void
PorousFlowSink::computeJacobian()
{
  _qp_map.assign(_test.size(), -1);
  for (unsigned qp = 0; qp < _qrule->n_points(); qp++)
    _qp_map[_node_number[qp]] = qp;
  IntegratedBC::computeJacobian();
}

void
PorousFlowSink::computeJacobianBlock(unsigned int jvar)
{
  _qp_map.assign(_test.size(), -1);
  for (unsigned qp = 0; qp < _qrule->n_points(); qp++)
    _qp_map[_node_number[qp]] = qp;
  IntegratedBC::computeJacobianBlock(jvar);
}

Real
PorousFlowSink::computeQpJacobian()
{
  return jac(_var.number());
}

Real
PorousFlowSink::computeQpOffDiagJacobian(unsigned int jvar)
{
  return jac(jvar);
}

Real
PorousFlowSink::jac(unsigned int jvar)
{
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;
  if (_qp_map[_i] == -1) // the Residual is zero for these nodes
    return 0.0;
  const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);
  const int qp_for_this_node = _qp_map[_i];

  Real flux = 0;
  Real deriv = 0;

  if (_i != _j)
  {
    // since the only non-upwinded contribution to the residual is
    // from the permeability, the only contribution of the residual
    // at node _i from changing jvar at node _j is through the derivative
    // of permeability
    if (!_use_mobility)
      return 0.0;
    deriv = _test[_i][_qp] * multiplier();
    const Real kprime = (_dpermeability_dvar[_qp][pvar] * _normals[_qp]) * _normals[_qp];
    deriv *= _fluid_density_node[qp_for_this_node][_ph] * kprime / _fluid_viscosity[qp_for_this_node][_ph];
    if (_use_relperm)
      deriv *= _relative_permeability[qp_for_this_node][_ph];
    if (_use_mass_fraction)
      deriv *= _mass_fractions[qp_for_this_node][_ph][_sp];
    return deriv * _phi[_j][_qp];
  }

  flux = _test[_i][_qp] * multiplier();
  deriv = _test[_i][_qp] * dmultiplier_dvar(pvar);
  if (_use_mobility)
  {
    const Real k = (_permeability[_qp] * _normals[_qp]) * _normals[_qp];
    const Real mob = _fluid_density_node[qp_for_this_node][_ph] * k / _fluid_viscosity[qp_for_this_node][_ph];
    const Real kprime = (_dpermeability_dvar[_qp][pvar] * _normals[_qp]) * _normals[_qp] * _phi[_j][_qp];
    const Real mobprime = _dfluid_density_node_dvar[qp_for_this_node][_ph][pvar] * k / _fluid_viscosity[qp_for_this_node][_ph] + _fluid_density_node[qp_for_this_node][_ph] * kprime / _fluid_viscosity[qp_for_this_node][_ph] - _fluid_density_node[qp_for_this_node][_ph] * k * _dfluid_viscosity_dvar[qp_for_this_node][_ph][pvar] / std::pow(_fluid_viscosity[qp_for_this_node][_ph], 2);
    deriv = mob * deriv + mobprime * flux;
    flux *= mob;
  }
  if (_use_relperm)
  {
    deriv = _relative_permeability[qp_for_this_node][_ph] * deriv + _drelative_permeability_dvar[qp_for_this_node][_ph][pvar] * flux;
    flux *= _relative_permeability[qp_for_this_node][_ph];
  }
  if (_use_mass_fraction)
  {
    deriv = _mass_fractions[qp_for_this_node][_ph][_sp] * deriv + _dmass_fractions_dvar[qp_for_this_node][_ph][_sp][pvar] * flux;
    flux *= _mass_fractions[qp_for_this_node][_ph][_sp];
  }
  return deriv;
}

Real
PorousFlowSink::multiplier()
{
  return _m_func.value(_t, _q_point[_qp]);
}

Real
PorousFlowSink::dmultiplier_dvar(unsigned int /*pvar*/)
{
  return 0.0;
}
