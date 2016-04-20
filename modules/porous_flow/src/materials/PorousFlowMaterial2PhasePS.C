/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterial2PhasePS.h"

template<>
InputParameters validParams<PorousFlowMaterial2PhasePS>()
{
  InputParameters params = validParams<PorousFlowStateBase>();

  params.addRequiredCoupledVar("phase0_porepressure", "Variable that is the porepressure of phase 0 (eg, the gas phase)");
  params.addRequiredCoupledVar("phase1_saturation", "Variable that is the saturation of phase 1 (eg, the water phase)");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations in a 2-phase isothermal situation, and derivatives of these with respect to the PorousFlowVariables.");
  return params;
}

PorousFlowMaterial2PhasePS::PorousFlowMaterial2PhasePS(const InputParameters & parameters) :
    PorousFlowStateBase(parameters),

    _num_ph(2),
    _phase0_porepressure_nodal(coupledNodalValue("phase0_porepressure")),
    _phase0_porepressure_qp(coupledValue("phase0_porepressure")),
    _phase0_gradp_qp(coupledGradient("phase0_porepressure")),
    _phase0_porepressure_varnum(coupled("phase0_porepressure")),

    _phase1_saturation_nodal(coupledNodalValue("phase1_saturation")),
    _phase1_saturation_qp(coupledValue("phase1_saturation")),
    _phase1_grads_qp(coupledGradient("phase1_saturation")),
    _phase1_saturation_varnum(coupled("phase1_saturation")),

    _phase0_temperature_nodal(coupledNodalValue("phase0_temperature")),
    _phase0_temperature_qp(coupledValue("phase0_temperature")),
    _phase0_temperature_varnum(coupled("phase0_temperature"))
{
  if (_dictator_UO.num_phases() != _num_ph)
    mooseError("The Dictator proclaims that the number of phases is " << _dictator_UO.num_phases() << " whereas PorousFlowMaterial2PhasePS can only be used for 2-phase simulation.  Be aware that the Dictator has noted your mistake.");
}

void
PorousFlowMaterial2PhasePS::initQpStatefulProperties()
{
  _porepressure_nodal[_qp].resize(_num_ph);
  _porepressure_qp[_qp].resize(_num_ph);
  _porepressure_nodal_old[_qp].resize(_num_ph);
  _gradp_qp[_qp].resize(_num_ph);
  _dporepressure_nodal_dvar[_qp].resize(_num_ph);
  _dporepressure_qp_dvar[_qp].resize(_num_ph);
  _dgradp_qp_dgradv[_qp].resize(_num_ph);
  _dgradp_qp_dv[_qp].resize(_num_ph);

  _saturation_nodal[_qp].resize(_num_ph);
  _saturation_qp[_qp].resize(_num_ph);
  _saturation_nodal_old[_qp].resize(_num_ph);
  _grads_qp[_qp].resize(_num_ph);
  _dsaturation_nodal_dvar[_qp].resize(_num_ph);
  _dsaturation_qp_dvar[_qp].resize(_num_ph);
  _dgrads_qp_dgradv[_qp].resize(_num_ph);
  _dgrads_qp_dv[_qp].resize(_num_ph);

  _temperature_nodal[_qp].resize(_num_ph);
  _temperature_qp[_qp].resize(_num_ph);
  _dtemperature_nodal_dvar[_qp].resize(_num_ph);
  _dtemperature_qp_dvar[_qp].resize(_num_ph);

  /*
   *  YAQI HACK !!
   *
   * I really just want to simply call
   * buildQpPPSS();
   * but i do it below in computeQpProperties
   */
   buildQpPPSSTT();
}

void
PorousFlowMaterial2PhasePS::computeQpProperties()
{
  buildQpPPSSTT();

  /*
   *  YAQI HACK !!
   *
   * I really just want to simply call
   * buildQpPPSS();
   * from initQpStatefulProperties, but the Variables
   * aren't initialised at that point so moose crashes
   */
  /*  if (_t_step == 1)
    for (unsigned ph = 0; ph < 2; ++ph)
    {
      _porepressure_old[_qp][ph] = _porepressure[_qp][ph];
      _saturation_old[_qp][ph] = _saturation[_qp][ph];
    }
  */

  /*
   * TODO: these derivatives could be put into the initQpStatefulProperties
   *       but only if i keep pc=constant, which is probably unphysical, so i'll
   *       keep the following computations here for modification with
   *       a physical pc later.
   */
  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_ph; ++phase)
  {
    _dporepressure_nodal_dvar[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
    _dporepressure_qp_dvar[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
    _dgradp_qp_dgradv[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
    _dgradp_qp_dv[_qp][phase].assign(_dictator_UO.num_v(), RealGradient());
  }

  // _porepressure is only dependent on _phase0_porepressure, and its derivative is 1
  if (!(_dictator_UO.not_porflow_var(_phase0_porepressure_varnum)))
  {
    // _phase0_porepressure is a porflow variable
    for (unsigned phase = 0; phase < _num_ph; ++phase)
    {
      _dporepressure_nodal_dvar[_qp][phase][_dictator_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1;
      _dporepressure_qp_dvar[_qp][phase][_dictator_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1;
      _dgradp_qp_dgradv[_qp][phase][_dictator_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1;
    }
  }

  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_ph; ++phase)
  {
    _dsaturation_nodal_dvar[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
    _dsaturation_qp_dvar[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
    _dgrads_qp_dgradv[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
  }

  // _saturation is only dependent on _phase1_saturation, and its derivative is +/- 1
  if (!(_dictator_UO.not_porflow_var(_phase1_saturation_varnum)))
  {
    // _phase1_saturation is a porflow variable
    _dsaturation_nodal_dvar[_qp][0][_dictator_UO.porflow_var_num(_phase1_saturation_varnum)] = -1.0;
    _dsaturation_nodal_dvar[_qp][1][_dictator_UO.porflow_var_num(_phase1_saturation_varnum)] = 1.0;
    _dsaturation_qp_dvar[_qp][0][_dictator_UO.porflow_var_num(_phase1_saturation_varnum)] = -1.0;
    _dsaturation_qp_dvar[_qp][1][_dictator_UO.porflow_var_num(_phase1_saturation_varnum)] = 1.0;
    _dgrads_qp_dgradv[_qp][0][_dictator_UO.porflow_var_num(_phase1_saturation_varnum)] = -1.0;
    _dgrads_qp_dgradv[_qp][1][_dictator_UO.porflow_var_num(_phase1_saturation_varnum)] = 1.0;
  }

  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_ph; ++phase)
  {
    _dtemperature_nodal_dvar[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
    _dtemperature_qp_dvar[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
  }

  // _temperature is only dependent on _phase0_temperature, and its derivative is = 1
  if (!(_dictator_UO.not_porflow_var(_phase0_temperature_varnum)))
  {
    // _phase0_temperature is a porflow variable
    _dtemperature_nodal_dvar[_qp][0][_dictator_UO.porflow_var_num(_phase0_temperature_varnum)] = 1.0;
    _dtemperature_nodal_dvar[_qp][1][_dictator_UO.porflow_var_num(_phase0_temperature_varnum)] = 1.0;
    _dtemperature_qp_dvar[_qp][0][_dictator_UO.porflow_var_num(_phase0_temperature_varnum)] = 1.0;
    _dtemperature_qp_dvar[_qp][1][_dictator_UO.porflow_var_num(_phase0_temperature_varnum)] = 1.0;
  }
}

void
PorousFlowMaterial2PhasePS::buildQpPPSSTT()
{
  const Real pc_nodal = 1.0;  // TODO: read in capillary suction function material
  const Real pc_qp = 1.0;
  _porepressure_nodal[_qp][0] = _phase0_porepressure_nodal[_qp];
  _porepressure_nodal[_qp][1] = _phase0_porepressure_nodal[_qp] - pc_nodal;
  _porepressure_qp[_qp][0] = _phase0_porepressure_qp[_qp];
  _porepressure_qp[_qp][1] = _phase0_porepressure_qp[_qp] - pc_qp;
  _gradp_qp[_qp][0] = _phase0_gradp_qp[_qp];
  _gradp_qp[_qp][1] = _phase0_gradp_qp[_qp];

  _saturation_nodal[_qp][0] = 1.0 - _phase1_saturation_nodal[_qp];
  _saturation_nodal[_qp][1] = _phase1_saturation_nodal[_qp];
  _saturation_qp[_qp][0] = 1.0 - _phase1_saturation_qp[_qp];
  _saturation_qp[_qp][1] = _phase1_saturation_qp[_qp];
  _grads_qp[_qp][0] = -_phase1_grads_qp[_qp];
  _grads_qp[_qp][1] = _phase1_grads_qp[_qp];

  /// Temperature is the same in each phase presently
  _temperature_nodal[_qp][0] = _phase0_temperature_nodal[_qp];
  _temperature_nodal[_qp][1] = _phase0_temperature_nodal[_qp];
  _temperature_qp[_qp][0] = _phase0_temperature_qp[_qp];
  _temperature_qp[_qp][1] = _phase0_temperature_qp[_qp];
}
