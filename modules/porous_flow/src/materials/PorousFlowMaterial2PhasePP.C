/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMaterial2PhasePP.h"

template<>
InputParameters validParams<PorousFlowMaterial2PhasePP>()
{
  InputParameters params = validParams<PorousFlowVariableBase>();

  params.addRequiredCoupledVar("phase0_porepressure", "Variable that is the porepressure of phase 0 (eg, the water phase).  It will be <= phase1_porepressure.");
  params.addRequiredCoupledVar("phase1_porepressure", "Variable that is the porepressure of phase 1 (eg, the gas phase)");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations in a 2-phase isothermal situation, and derivatives of these with respect to the PorousFlowVariables");
  return params;
}

PorousFlowMaterial2PhasePP::PorousFlowMaterial2PhasePP(const InputParameters & parameters) :
    PorousFlowVariableBase(parameters),

    _phase0_porepressure_nodal(coupledNodalValue("phase0_porepressure")),
    _phase0_porepressure_qp(coupledValue("phase0_porepressure")),
    _phase0_gradp_qp(coupledGradient("phase0_porepressure")),
    _phase0_porepressure_varnum(coupled("phase0_porepressure")),

    _phase1_porepressure_nodal(coupledNodalValue("phase1_porepressure")),
    _phase1_porepressure_qp(coupledValue("phase1_porepressure")),
    _phase1_gradp_qp(coupledGradient("phase1_porepressure")),
    _phase1_porepressure_varnum(coupled("phase1_porepressure"))
{
  if (_dictator_UO.numPhases() != 2)
    mooseError("The Dictator announces that the number of phases is " << _dictator_UO.numPhases() << " whereas PorousFlowMaterial2PhasePP can only be used for 2-phase simulation.  When you have an efficient government, you have a dictatorship.");
}

void
PorousFlowMaterial2PhasePP::initQpStatefulProperties()
{
  PorousFlowVariableBase::initQpStatefulProperties();

  buildQpPPSS();

  /*
   * the derivatives of porepressure with respect to porepressure
   * remain fixed (at unity) throughout the simulation
   */
  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_phases; ++phase)
  {
    _dporepressure_nodal_dvar[_qp][phase].assign(_dictator_UO.numVariables(), 0.0);
    _dporepressure_qp_dvar[_qp][phase].assign(_dictator_UO.numVariables(), 0.0);
    _dgradp_qp_dgradv[_qp][phase].assign(_dictator_UO.numVariables(), 0.0);
    _dgradp_qp_dv[_qp][phase].assign(_dictator_UO.numVariables(), RealGradient());
  }

  if (!_dictator_UO.not_porflow_var(_phase0_porepressure_varnum))
  {
    // _phase0_porepressure is a PorousFlow variable
    _dporepressure_nodal_dvar[_qp][0][_dictator_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1.0;
    _dporepressure_qp_dvar[_qp][0][_dictator_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1.0;
    _dgradp_qp_dgradv[_qp][0][_dictator_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1.0;
  }
  if (!_dictator_UO.not_porflow_var(_phase1_porepressure_varnum))
  {
    // _phase1_porepressure is a PorousFlow variable
    _dporepressure_nodal_dvar[_qp][1][_dictator_UO.porflow_var_num(_phase1_porepressure_varnum)] = 1.0;
    _dporepressure_qp_dvar[_qp][1][_dictator_UO.porflow_var_num(_phase1_porepressure_varnum)] = 1.0;
    _dgradp_qp_dgradv[_qp][1][_dictator_UO.porflow_var_num(_phase1_porepressure_varnum)] = 1.0;
  }

  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_phases; ++phase)
  {
    _dtemperature_nodal_dvar[_qp][phase].assign(_dictator_UO.numVariables(), 0.0);
    _dtemperature_qp_dvar[_qp][phase].assign(_dictator_UO.numVariables(), 0.0);
  }

  // _temperature is only dependent on _temperature, and its derivative is = 1
  if (!_dictator_UO.not_porflow_var(_temperature_varnum))
  {
    // _phase0_temperature is a porflow variable
    _dtemperature_nodal_dvar[_qp][0][_dictator_UO.porflow_var_num(_temperature_varnum)] = 1.0;
    _dtemperature_nodal_dvar[_qp][1][_dictator_UO.porflow_var_num(_temperature_varnum)] = 1.0;
    _dtemperature_qp_dvar[_qp][0][_dictator_UO.porflow_var_num(_temperature_varnum)] = 1.0;
    _dtemperature_qp_dvar[_qp][1][_dictator_UO.porflow_var_num(_temperature_varnum)] = 1.0;
  }
}

void
PorousFlowMaterial2PhasePP::computeQpProperties()
{
  buildQpPPSS();

  /* The derivatives of saturation with respect to
   * porepressure depend on porepressure
   */

  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_phases; ++phase)
  {
    _dsaturation_nodal_dvar[_qp][phase].assign(_dictator_UO.numVariables(), 0.0);
    _dsaturation_qp_dvar[_qp][phase].assign(_dictator_UO.numVariables(), 0.0);
    _dgrads_qp_dgradv[_qp][phase].assign(_dictator_UO.numVariables(), 0.0);
    _dgrads_qp_dv[_qp][phase].assign(_dictator_UO.numVariables(), RealGradient());
  }

  const Real pc_nodal = _phase0_porepressure_nodal[_qp] - _phase1_porepressure_nodal[_qp]; // this is <= 0
  const Real dseff_nodal = dEffectiveSaturation_dP(pc_nodal); // d(seff)/d(pc)
  const Real pc_qp = _phase0_porepressure_qp[_qp] - _phase1_porepressure_qp[_qp]; // this is <= 0
  const Real dseff_qp = dEffectiveSaturation_dP(pc_qp); // d(seff_qp)/d(pc_qp)
  const Real d2seff_qp = d2EffectiveSaturation_dP2(pc_qp); // d^2(seff_qp)/d(pc_qp)^2

  if (!_dictator_UO.not_porflow_var(_phase0_porepressure_varnum))
  {
    const unsigned int pvar = _dictator_UO.porflow_var_num(_phase0_porepressure_varnum);
    _dsaturation_nodal_dvar[_qp][0][pvar] = dseff_nodal;
    _dsaturation_qp_dvar[_qp][0][pvar] = dseff_qp;
    _dgrads_qp_dgradv[_qp][0][pvar] = dseff_qp;
    _dgrads_qp_dv[_qp][0][pvar] = d2seff_qp * (_phase0_gradp_qp[_qp] - _phase1_gradp_qp[_qp]);

    _dsaturation_nodal_dvar[_qp][1][pvar] = - dseff_nodal;
    _dsaturation_qp_dvar[_qp][1][pvar] = - dseff_qp;
    _dgrads_qp_dgradv[_qp][1][pvar] = - dseff_qp;
    _dgrads_qp_dv[_qp][1][pvar] = - d2seff_qp * (_phase0_gradp_qp[_qp] - _phase1_gradp_qp[_qp]);
  }

  if (!_dictator_UO.not_porflow_var(_phase1_porepressure_varnum))
  {
    const unsigned int pvar = _dictator_UO.porflow_var_num(_phase1_porepressure_varnum);
    _dsaturation_nodal_dvar[_qp][0][pvar] = - dseff_nodal;
    _dsaturation_qp_dvar[_qp][0][pvar] = - dseff_qp;
    _dgrads_qp_dgradv[_qp][0][pvar] = - dseff_qp;
    _dgrads_qp_dv[_qp][0][pvar] = - d2seff_qp * (_phase0_gradp_qp[_qp] - _phase1_gradp_qp[_qp]);

    _dsaturation_nodal_dvar[_qp][1][pvar] = dseff_nodal;
    _dsaturation_qp_dvar[_qp][1][pvar] = dseff_qp;
    _dgrads_qp_dgradv[_qp][1][pvar] = dseff_qp;
    _dgrads_qp_dv[_qp][1][pvar] = d2seff_qp * (_phase0_gradp_qp[_qp] - _phase1_gradp_qp[_qp]);
  }
}

void
PorousFlowMaterial2PhasePP::buildQpPPSS()
{
  _porepressure_nodal[_qp][0] = _phase0_porepressure_nodal[_qp];
  _porepressure_nodal[_qp][1] = _phase1_porepressure_nodal[_qp];
  _porepressure_qp[_qp][0] = _phase0_porepressure_qp[_qp];
  _porepressure_qp[_qp][1] = _phase1_porepressure_qp[_qp];
  _gradp_qp[_qp][0] = _phase0_gradp_qp[_qp];
  _gradp_qp[_qp][1] = _phase1_gradp_qp[_qp];

  const Real pc_nodal = _phase0_porepressure_nodal[_qp] - _phase1_porepressure_nodal[_qp]; // this is <= 0
  const Real seff_nodal = effectiveSaturation(pc_nodal);

  const Real pc_qp = _phase0_porepressure_qp[_qp] - _phase1_porepressure_qp[_qp]; // this is <= 0
  const Real seff_qp = effectiveSaturation(pc_qp);
  const Real dseff_qp = dEffectiveSaturation_dP(pc_qp); // d(seff_qp)/d(pc_qp)

  _saturation_nodal[_qp][0] = seff_nodal;
  _saturation_nodal[_qp][1] = 1.0 - seff_nodal;
  _saturation_qp[_qp][0] = seff_qp;
  _saturation_qp[_qp][1] = 1.0 - seff_qp;
  _grads_qp[_qp][0] = dseff_qp * (_gradp_qp[_qp][0] - _gradp_qp[_qp][1]);
  _grads_qp[_qp][1] = - _grads_qp[_qp][0];

  /// Temperature is the same in each phase presently
  _temperature_nodal[_qp][0] = _temperature_nodal_var[_qp];
  _temperature_nodal[_qp][1] = _temperature_nodal_var[_qp];
  _temperature_qp[_qp][0] = _temperature_qp_var[_qp];
  _temperature_qp[_qp][1] = _temperature_qp_var[_qp];
}

Real
PorousFlowMaterial2PhasePP::effectiveSaturation(Real /* pressure */) const
{
  return 1.0;
}

Real
PorousFlowMaterial2PhasePP::dEffectiveSaturation_dP(Real /* pressure */) const
{
  return 0.0;
}

Real
PorousFlowMaterial2PhasePP::d2EffectiveSaturation_dP2(Real /* pressure */) const
{
  return 0.0;
}
