//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFullySaturatedDarcyBase.h"

#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowFullySaturatedDarcyBase);

InputParameters
PorousFlowFullySaturatedDarcyBase::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<RealVectorValue>("gravity",
                                           "Gravitational acceleration vector downwards (m/s^2)");
  params.addParam<bool>("multiply_by_density",
                        true,
                        "If true, then this Kernel is the fluid mass "
                        "flux.  If false, then this Kernel is the "
                        "fluid volume flux (which is common in "
                        "poro-mechanics)");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addClassDescription("Darcy flux suitable for models involving a fully-saturated, single "
                             "phase, single component fluid.  No upwinding is used");
  return params;
}

PorousFlowFullySaturatedDarcyBase::PorousFlowFullySaturatedDarcyBase(
    const InputParameters & parameters)
  : Kernel(parameters),
    _multiply_by_density(getParam<bool>("multiply_by_density")),
    _permeability(getMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _dpermeability_dvar(
        getMaterialProperty<std::vector<RealTensorValue>>("dPorousFlow_permeability_qp_dvar")),
    _dpermeability_dgradvar(getMaterialProperty<std::vector<std::vector<RealTensorValue>>>(
        "dPorousFlow_permeability_qp_dgradvar")),
    _density(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _ddensity_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_density_qp_dvar")),
    _viscosity(getMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_qp")),
    _dviscosity_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_viscosity_qp_dvar")),
    _pp(getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _grad_p(getMaterialProperty<std::vector<RealGradient>>("PorousFlow_grad_porepressure_qp")),
    _dgrad_p_dgrad_var(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_grad_porepressure_qp_dgradvar")),
    _dgrad_p_dvar(getMaterialProperty<std::vector<std::vector<RealGradient>>>(
        "dPorousFlow_grad_porepressure_qp_dvar")),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _perm_derivs(_dictator.usePermDerivs())
{
  if (_dictator.numPhases() != 1)
    mooseError("PorousFlowFullySaturatedDarcyBase should not be used for multi-phase scenarios as "
               "it does no upwinding and does not include relative-permeability effects");
}

Real
PorousFlowFullySaturatedDarcyBase::computeQpResidual()
{
  const unsigned ph = 0;
  const Real mob = mobility();
  const RealVectorValue flow =
      _permeability[_qp] * (_grad_p[_qp][ph] - _density[_qp][ph] * _gravity);
  return _grad_test[_i][_qp] * mob * flow;
}

Real
PorousFlowFullySaturatedDarcyBase::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
PorousFlowFullySaturatedDarcyBase::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;

  const unsigned ph = 0;
  const unsigned pvar = _dictator.porousFlowVariableNum(jvar);

  const Real mob = mobility();
  const Real dmob = dmobility(pvar) * _phi[_j][_qp];

  const RealVectorValue flow =
      _permeability[_qp] * (_grad_p[_qp][ph] - _density[_qp][ph] * _gravity);

  RealVectorValue dflow =
      _permeability[_qp] * (_grad_phi[_j][_qp] * _dgrad_p_dgrad_var[_qp][ph][pvar] -
                            _phi[_j][_qp] * _ddensity_dvar[_qp][ph][pvar] * _gravity);
  dflow += _permeability[_qp] * (_dgrad_p_dvar[_qp][ph][pvar] * _phi[_j][_qp]);

  if (_perm_derivs)
  {
    dflow += _dpermeability_dvar[_qp][pvar] * _phi[_j][_qp] *
             (_grad_p[_qp][ph] - _density[_qp][ph] * _gravity);
    for (const auto i : make_range(Moose::dim))
      dflow += _dpermeability_dgradvar[_qp][i][pvar] * _grad_phi[_j][_qp](i) *
               (_grad_p[_qp][ph] - _density[_qp][ph] * _gravity);
  }

  return _grad_test[_i][_qp] * (dmob * flow + mob * dflow);
}

Real
PorousFlowFullySaturatedDarcyBase::mobility() const
{
  const unsigned ph = 0;
  Real mob = 1.0 / _viscosity[_qp][ph];
  if (_multiply_by_density)
    mob *= _density[_qp][ph];
  return mob;
}

Real
PorousFlowFullySaturatedDarcyBase::dmobility(unsigned pvar) const
{
  const unsigned ph = 0;
  Real dmob = -_dviscosity_dvar[_qp][ph][pvar] / std::pow(_viscosity[_qp][ph], 2);
  if (_multiply_by_density)
    dmob = _density[_qp][ph] * dmob + _ddensity_dvar[_qp][ph][pvar] / _viscosity[_qp][ph];
  return dmob;
}
