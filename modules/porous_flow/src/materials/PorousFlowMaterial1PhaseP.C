/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterial1PhaseP.h"

template<>
InputParameters validParams<PorousFlowMaterial1PhaseP>()
{
  InputParameters params = validParams<PorousFlowVariableBase>();

  params.addRequiredCoupledVar("porepressure", "Variable that represents the porepressure of the single phase");
  params.addClassDescription("This Material is used for the fully saturated single-phase situation where porepressure is the primary variable");
  return params;
}

PorousFlowMaterial1PhaseP::PorousFlowMaterial1PhaseP(const InputParameters & parameters) :
    PorousFlowVariableBase(parameters),

    _porepressure_nodal_var(coupledNodalValue("porepressure")),
    _porepressure_qp_var(coupledValue("porepressure")),
    _gradp_qp_var(coupledGradient("porepressure")),
    _porepressure_varnum(coupled("porepressure"))
{
  if (_dictator_UO.num_phases() != 1)
    mooseError("The Dictator proclaims that the number of phases is " << _dictator_UO.num_phases() << " whereas PorousFlowMaterial1PhaseP can only be used for 1-phase simulations.  Be aware that the Dictator has noted your mistake.");
}

void
PorousFlowMaterial1PhaseP::initQpStatefulProperties()
{
  PorousFlowVariableBase::initQpStatefulProperties();

  /*
   *  YAQI HACK !!
   *
   * I really just want to simply call
   * buildQpPPSS();
   * but i do it below in computeQpProperties
   */
  buildQpPPSS();
}

void
PorousFlowMaterial1PhaseP::computeQpProperties()
{
  buildQpPPSS();

  /*
   *  YAQI HACK !!
   *
   * I really just want to simply call
   * buildQpPPSS();
   * from initQpStatefulProperties, but the Variables
   * aren't initialised at that point so moose crashes
   */
  /*if (_t_step == 1)
    for (unsigned ph = 0; ph < _num_ph; ++ph)
    {
      _porepressure_old[_qp][ph] = _porepressure[_qp][ph];
      _saturation_old[_qp][ph] = _saturation[_qp][ph];
    }
  */

  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_phases; ++phase)
  {
    _dporepressure_nodal_dvar[_qp][phase].assign(_dictator_UO.num_variables(), 0.0);
    _dporepressure_qp_dvar[_qp][phase].assign(_dictator_UO.num_variables(), 0.0);
    _dgradp_qp_dgradv[_qp][phase].assign(_dictator_UO.num_variables(), 0.0);
    _dgradp_qp_dv[_qp][phase].assign(_dictator_UO.num_variables(), RealGradient());
  }

  // _porepressure is only dependent on _porepressure, and its derivative is 1
  if (!(_dictator_UO.not_porflow_var(_porepressure_varnum)))
  {
    // _porepressure is a PorousFlow variable
    _dporepressure_nodal_dvar[_qp][0][_dictator_UO.porflow_var_num(_porepressure_varnum)] = 1;
    _dporepressure_qp_dvar[_qp][0][_dictator_UO.porflow_var_num(_porepressure_varnum)] = 1;
    _dgradp_qp_dgradv[_qp][0][_dictator_UO.porflow_var_num(_porepressure_varnum)] = 1;
  }


  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_phases; ++phase)
  {
    _dsaturation_nodal_dvar[_qp][phase].assign(_dictator_UO.num_variables(), 0.0);
    _dsaturation_qp_dvar[_qp][phase].assign(_dictator_UO.num_variables(), 0.0);
    _dgrads_qp_dgradv[_qp][phase].assign(_dictator_UO.num_variables(), 0.0);
    _dgrads_qp_dv[_qp][phase].assign(_dictator_UO.num_variables(), RealGradient());
  }

  if (!(_dictator_UO.not_porflow_var(_porepressure_varnum)))
  {
    // _porepressure is a porflow variable
    _dsaturation_nodal_dvar[_qp][0][_dictator_UO.porflow_var_num(_porepressure_varnum)] = dEffectiveSaturation_dP(_porepressure_nodal_var[_qp]);
    _dsaturation_qp_dvar[_qp][0][_dictator_UO.porflow_var_num(_porepressure_varnum)] = dEffectiveSaturation_dP(_porepressure_qp_var[_qp]);
    _dgrads_qp_dgradv[_qp][0][_dictator_UO.porflow_var_num(_porepressure_varnum)] = dEffectiveSaturation_dP(_porepressure_qp_var[_qp]);
    _dgrads_qp_dv[_qp][0][_dictator_UO.porflow_var_num(_porepressure_varnum)] = d2EffectiveSaturation_dP2(_porepressure_qp_var[_qp]) * _gradp_qp_var[_qp];
  }

  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_phases; ++phase)
  {
    _dtemperature_nodal_dvar[_qp][phase].assign(_dictator_UO.num_variables(), 0.0);
    _dtemperature_qp_dvar[_qp][phase].assign(_dictator_UO.num_variables(), 0.0);
  }

  // _temperature is only dependent on _temperature, and its derivative is = 1
  if (!(_dictator_UO.not_porflow_var(_temperature_varnum)))
  {
    // _temperature is a porflow variable
    _dtemperature_nodal_dvar[_qp][0][_dictator_UO.porflow_var_num(_temperature_varnum)] = 1.0;
    _dtemperature_qp_dvar[_qp][0][_dictator_UO.porflow_var_num(_temperature_varnum)] = 1.0;
  }
}

void
PorousFlowMaterial1PhaseP::buildQpPPSS()
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
PorousFlowMaterial1PhaseP::effectiveSaturation(Real /* pressure */) const
{
  return 1.0;
}

Real
PorousFlowMaterial1PhaseP::dEffectiveSaturation_dP(Real /* pressure */) const
{
  return 0.0;
}

Real
PorousFlowMaterial1PhaseP::d2EffectiveSaturation_dP2(Real /* pressure */) const
{
  return 0.0;
}
