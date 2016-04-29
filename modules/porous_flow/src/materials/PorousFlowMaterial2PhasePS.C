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
  InputParameters params = validParams<PorousFlowVariableBase>();
  params.addRequiredCoupledVar("phase0_porepressure", "Variable that is the porepressure of phase 0 (eg, the gas phase)");
  params.addRequiredCoupledVar("phase1_saturation", "Variable that is the saturation of phase 1 (eg, the water phase)");
  params.addParam<Real>("pc", 0.0, "Constant capillary pressure (Pa). Default is 0.0");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations in a 2-phase isothermal situation, and derivatives of these with respect to the PorousFlowVariables.");
  return params;
}

PorousFlowMaterial2PhasePS::PorousFlowMaterial2PhasePS(const InputParameters & parameters) :
    PorousFlowVariableBase(parameters),

    _phase0_porepressure_nodal(coupledNodalValue("phase0_porepressure")),
    _phase0_porepressure_qp(coupledValue("phase0_porepressure")),
    _phase0_gradp_qp(coupledGradient("phase0_porepressure")),
    _phase0_porepressure_varnum(coupled("phase0_porepressure")),

    _phase1_saturation_nodal(coupledNodalValue("phase1_saturation")),
    _phase1_saturation_qp(coupledValue("phase1_saturation")),
    _phase1_grads_qp(coupledGradient("phase1_saturation")),
    _phase1_saturation_varnum(coupled("phase1_saturation")),

    _pc(getParam<Real>("pc"))
{
  if (_dictator_UO.numPhases() != 2)
    mooseError("The Dictator proclaims that the number of phases is " << _dictator_UO.numPhases() << " whereas PorousFlowMaterial2PhasePS can only be used for 2-phase simulation.  Be aware that the Dictator has noted your mistake.");
}

void
PorousFlowMaterial2PhasePS::initQpStatefulProperties()
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
PorousFlowMaterial2PhasePS::computeQpProperties()
{
  PorousFlowVariableBase::computeQpProperties();

  buildQpPPSS();

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

  // _porepressure depends on _phase0_porepressure, and its derivative is 1
  if (!_dictator_UO.not_porflow_var(_phase0_porepressure_varnum))
  {
    // _phase0_porepressure is a PorousFlow variable
    for (unsigned phase = 0; phase < _num_phases; ++phase)
    {
      _dporepressure_nodal_dvar[_qp][phase][_dictator_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1.0;
      _dporepressure_qp_dvar[_qp][phase][_dictator_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1.0;
      _dgradp_qp_dgradv[_qp][phase][_dictator_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1.0;
    }
  }

  /// Calculate the capillary pressure and derivatives wrt saturation
  const Real dpc_nodal = dCapillaryPressure_dS(_phase1_saturation_nodal[_qp]);
  const Real dpc_qp = dCapillaryPressure_dS(_phase1_saturation_qp[_qp]);
  const Real d2pc_qp = d2CapillaryPressure_dS2(_phase1_saturation_qp[_qp]);

  // _saturation is only dependent on _phase1_saturation, and its derivative is +/- 1
  if (!_dictator_UO.not_porflow_var(_phase1_saturation_varnum))
  {
    const unsigned int svar = _dictator_UO.porflow_var_num(_phase1_saturation_varnum);
    // _phase1_saturation is a porflow variable
    _dsaturation_nodal_dvar[_qp][0][svar] = -1.0;
    _dsaturation_nodal_dvar[_qp][1][svar] = 1.0;
    _dsaturation_qp_dvar[_qp][0][svar] = -1.0;
    _dsaturation_qp_dvar[_qp][1][svar] = 1.0;
    _dgrads_qp_dgradv[_qp][0][svar] = -1.0;
    _dgrads_qp_dgradv[_qp][1][svar] = 1.0;

    /// _phase1_porepressure depends on saturation through the capillary pressure function
    _dporepressure_nodal_dvar[_qp][1][svar] = - dpc_nodal;
    _dporepressure_qp_dvar[_qp][1][svar] = - dpc_qp;
    _dgradp_qp_dv[_qp][1][svar] = - d2pc_qp * _grads_qp[_qp][1];
    _dgradp_qp_dgradv[_qp][1][svar] = - dpc_qp;
  }

  // _temperature is only dependent on temperature, and its derivative is = 1
  if (!_dictator_UO.not_porflow_var(_temperature_varnum))
  {
    // _phase0_temperature is a PorousFlow variable
    for (unsigned int phase = 0; phase < _num_phases; ++phase)
    {
      _dtemperature_nodal_dvar[_qp][phase][_dictator_UO.porflow_var_num(_temperature_varnum)] = 1.0;
      _dtemperature_qp_dvar[_qp][phase][_dictator_UO.porflow_var_num(_temperature_varnum)] = 1.0;
    }
  }
}

void
PorousFlowMaterial2PhasePS::buildQpPPSS()
{
  _saturation_nodal[_qp][0] = 1.0 - _phase1_saturation_nodal[_qp];
  _saturation_nodal[_qp][1] = _phase1_saturation_nodal[_qp];
  _saturation_qp[_qp][0] = 1.0 - _phase1_saturation_qp[_qp];
  _saturation_qp[_qp][1] = _phase1_saturation_qp[_qp];
  _grads_qp[_qp][0] = -_phase1_grads_qp[_qp];
  _grads_qp[_qp][1] = _phase1_grads_qp[_qp];

  const Real pc_nodal = capillaryPressure(_phase1_saturation_nodal[_qp]);
  const Real pc_qp = capillaryPressure(_phase1_saturation_qp[_qp]);
  const Real dpc_qp = dCapillaryPressure_dS(_phase1_saturation_qp[_qp]);

  _porepressure_nodal[_qp][0] = _phase0_porepressure_nodal[_qp];
  _porepressure_nodal[_qp][1] = _phase0_porepressure_nodal[_qp] - pc_nodal;
  _porepressure_qp[_qp][0] = _phase0_porepressure_qp[_qp];
  _porepressure_qp[_qp][1] = _phase0_porepressure_qp[_qp] - pc_qp;
  _gradp_qp[_qp][0] = _phase0_gradp_qp[_qp];
  _gradp_qp[_qp][1] = _phase0_gradp_qp[_qp] - dpc_qp * _grads_qp[_qp][1];

  /// Temperature is the same in each phase presently
  _temperature_nodal[_qp][0] = _temperature_nodal_var[_qp];
  _temperature_nodal[_qp][1] = _temperature_nodal_var[_qp];
  _temperature_qp[_qp][0] = _temperature_qp_var[_qp];
  _temperature_qp[_qp][1] = _temperature_qp_var[_qp];
}

Real
PorousFlowMaterial2PhasePS::capillaryPressure(Real /* saturation */) const
{
  return _pc;
}

Real
PorousFlowMaterial2PhasePS::dCapillaryPressure_dS(Real /* saturation */) const
{
  return 0.0;
}

Real
PorousFlowMaterial2PhasePS::d2CapillaryPressure_dS2(Real /* saturation */) const
{
  return 0.0;
}
