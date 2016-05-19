/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlow1PhaseP.h"

template<>
InputParameters validParams<PorousFlow1PhaseP>()
{
  InputParameters params = validParams<PorousFlowVariableBase>();

  params.addRequiredCoupledVar("porepressure", "Variable that represents the porepressure of the single phase");
  params.addClassDescription("This Material is used for the fully saturated single-phase situation where porepressure is the primary variable");
  return params;
}

PorousFlow1PhaseP::PorousFlow1PhaseP(const InputParameters & parameters) :
    PorousFlowVariableBase(parameters),

    _porepressure_nodal_var(coupledNodalValue("porepressure")),
    _porepressure_qp_var(coupledValue("porepressure")),
    _gradp_qp_var(coupledGradient("porepressure")),
    _porepressure_varnum(coupled("porepressure")),
    _p_var_num(_dictator.isPorousFlowVariable(_porepressure_varnum) ? _dictator.porousFlowVariableNum(_porepressure_varnum) : 0),
    _t_var_num(_dictator.isPorousFlowVariable(_temperature_varnum) ? _dictator.porousFlowVariableNum(_temperature_varnum) : 0)
{
  if (_dictator.numPhases() != 1)
    mooseError("The Dictator proclaims that the number of phases is " << _dictator.numPhases() << " whereas PorousFlow1PhaseP can only be used for 1-phase simulations.  Be aware that the Dictator has noted your mistake.");
}

void
PorousFlow1PhaseP::initQpStatefulProperties()
{
  PorousFlowVariableBase::initQpStatefulProperties();
  buildQpPPSS();
}

void
PorousFlow1PhaseP::computeQpProperties()
{
  buildQpPPSS();

  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_phases; ++phase)
  {
    _dporepressure_nodal_dvar[_qp][phase].assign(_num_pf_vars, 0.0);
    _dporepressure_qp_dvar[_qp][phase].assign(_num_pf_vars, 0.0);
    _dgradp_qp_dgradv[_qp][phase].assign(_num_pf_vars, 0.0);
    _dgradp_qp_dv[_qp][phase].assign(_num_pf_vars, RealGradient());
  }

  // _porepressure is only dependent on _porepressure, and its derivative is 1
  if (_dictator.isPorousFlowVariable(_porepressure_varnum))
  {
    // _porepressure is a PorousFlow variable
    _dporepressure_nodal_dvar[_qp][0][_p_var_num] = 1.0;
    _dporepressure_qp_dvar[_qp][0][_p_var_num] = 1.0;
    _dgradp_qp_dgradv[_qp][0][_p_var_num] = 1.0;
  }

  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_phases; ++phase)
  {
    _dsaturation_nodal_dvar[_qp][phase].assign(_num_pf_vars, 0.0);
    _dsaturation_qp_dvar[_qp][phase].assign(_num_pf_vars, 0.0);
    _dgrads_qp_dgradv[_qp][phase].assign(_num_pf_vars, 0.0);
    _dgrads_qp_dv[_qp][phase].assign(_num_pf_vars, RealGradient());
  }

  if (_dictator.isPorousFlowVariable(_porepressure_varnum))
  {
    // _porepressure is a porflow variable
    _dsaturation_nodal_dvar[_qp][0][_p_var_num] = dEffectiveSaturation_dP(_porepressure_nodal_var[_qp]);
    _dsaturation_qp_dvar[_qp][0][_p_var_num] = dEffectiveSaturation_dP(_porepressure_qp_var[_qp]);
    _dgrads_qp_dgradv[_qp][0][_p_var_num] = dEffectiveSaturation_dP(_porepressure_qp_var[_qp]);
    _dgrads_qp_dv[_qp][0][_p_var_num] = d2EffectiveSaturation_dP2(_porepressure_qp_var[_qp]) * _gradp_qp_var[_qp];
  }

  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_phases; ++phase)
  {
    _dtemperature_nodal_dvar[_qp][phase].assign(_num_pf_vars, 0.0);
    _dtemperature_qp_dvar[_qp][phase].assign(_num_pf_vars, 0.0);
  }

  // _temperature is only dependent on _temperature, and its derivative is = 1
  if (_dictator.isPorousFlowVariable(_temperature_varnum))
  {
    // _temperature is a porflow variable
    _dtemperature_nodal_dvar[_qp][0][_t_var_num] = 1.0;
    _dtemperature_qp_dvar[_qp][0][_t_var_num] = 1.0;
  }
}

void
PorousFlow1PhaseP::buildQpPPSS()
{
  _porepressure_nodal[_qp][0] = _porepressure_nodal_var[_qp];
  _porepressure_qp[_qp][0] = _porepressure_qp_var[_qp];
  _gradp_qp[_qp][0] = _gradp_qp_var[_qp];

  _saturation_nodal[_qp][0] = effectiveSaturation(_porepressure_nodal_var[_qp]);
  _saturation_qp[_qp][0] = effectiveSaturation(_porepressure_qp_var[_qp]);
  _grads_qp[_qp][0] = dEffectiveSaturation_dP(_porepressure_qp_var[_qp]) * _gradp_qp_var[_qp];

  /// Temperature is the same in each phase presently
  _temperature_nodal[_qp][0] = _temperature_nodal_var[_qp];
  _temperature_qp[_qp][0] = _temperature_qp_var[_qp];
}

Real
PorousFlow1PhaseP::effectiveSaturation(Real /* pressure */) const
{
  return 1.0;
}

Real
PorousFlow1PhaseP::dEffectiveSaturation_dP(Real /* pressure */) const
{
  return 0.0;
}

Real
PorousFlow1PhaseP::d2EffectiveSaturation_dP2(Real /* pressure */) const
{
  return 0.0;
}
