/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlow2PhasePS.h"

template<>
InputParameters validParams<PorousFlow2PhasePS>()
{
  InputParameters params = validParams<PorousFlowVariableBase>();
  params.addRequiredCoupledVar("phase0_porepressure", "Variable that is the porepressure of phase 0 (eg, the gas phase)");
  params.addRequiredCoupledVar("phase1_saturation", "Variable that is the saturation of phase 1 (eg, the water phase)");
  params.addParam<Real>("pc", 0.0, "Constant capillary pressure (Pa). Default is 0.0");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations in a 2-phase isothermal situation, and derivatives of these with respect to the PorousFlowVariables.");
  return params;
}

PorousFlow2PhasePS::PorousFlow2PhasePS(const InputParameters & parameters) :
    PorousFlowVariableBase(parameters),

    _phase0_porepressure_nodal(coupledNodalValue("phase0_porepressure")),
    _phase0_porepressure_qp(coupledValue("phase0_porepressure")),
    _phase0_gradp_qp(coupledGradient("phase0_porepressure")),
    _phase0_porepressure_varnum(coupled("phase0_porepressure")),
    _pvar(_dictator.isPorousFlowVariable(_phase0_porepressure_varnum) ? _dictator.porousFlowVariableNum(_phase0_porepressure_varnum) : 0),

    _phase1_saturation_nodal(coupledNodalValue("phase1_saturation")),
    _phase1_saturation_qp(coupledValue("phase1_saturation")),
    _phase1_grads_qp(coupledGradient("phase1_saturation")),
    _phase1_saturation_varnum(coupled("phase1_saturation")),
    _svar(_dictator.isPorousFlowVariable(_phase1_saturation_varnum) ? _dictator.porousFlowVariableNum(_phase1_saturation_varnum) : 0),

    _pc(getParam<Real>("pc"))

{
  if (_dictator.numPhases() != 2)
    mooseError("The Dictator proclaims that the number of phases is " << _dictator.numPhases() << " whereas PorousFlow2PhasePS can only be used for 2-phase simulation.  Be aware that the Dictator has noted your mistake.");
}

void
PorousFlow2PhasePS::initQpStatefulProperties()
{
  PorousFlowVariableBase::initQpStatefulProperties();

   buildQpPPSS();
}

void
PorousFlow2PhasePS::computeQpProperties()
{
  PorousFlowVariableBase::computeQpProperties();

  buildQpPPSS();

  // _porepressure depends on _phase0_porepressure, and its derivative is 1
  if (_dictator.isPorousFlowVariable(_phase0_porepressure_varnum))
  {
    // _phase0_porepressure is a PorousFlow variable
    for (unsigned phase = 0; phase < _num_phases; ++phase)
    {
      _dporepressure_nodal_dvar[_qp][phase][_pvar] = 1.0;
      _dporepressure_qp_dvar[_qp][phase][_pvar] = 1.0;
      _dgradp_qp_dgradv[_qp][phase][_pvar] = 1.0;
    }
  }

  /// Calculate the capillary pressure and derivatives wrt saturation
  const Real dpc_nodal = dCapillaryPressure_dS(_phase1_saturation_nodal[_node_number[_qp]]);
  const Real dpc_qp = dCapillaryPressure_dS(_phase1_saturation_qp[_qp]);
  const Real d2pc_qp = d2CapillaryPressure_dS2(_phase1_saturation_qp[_qp]);

  // _saturation is only dependent on _phase1_saturation, and its derivative is +/- 1
  if (_dictator.isPorousFlowVariable(_phase1_saturation_varnum))
  {
    // _phase1_saturation is a porflow variable
    _dsaturation_nodal_dvar[_qp][0][_svar] = -1.0;
    _dsaturation_nodal_dvar[_qp][1][_svar] = 1.0;
    _dsaturation_qp_dvar[_qp][0][_svar] = -1.0;
    _dsaturation_qp_dvar[_qp][1][_svar] = 1.0;
    _dgrads_qp_dgradv[_qp][0][_svar] = -1.0;
    _dgrads_qp_dgradv[_qp][1][_svar] = 1.0;

    /// _phase1_porepressure depends on saturation through the capillary pressure function
    _dporepressure_nodal_dvar[_qp][1][_svar] = - dpc_nodal;
    _dporepressure_qp_dvar[_qp][1][_svar] = - dpc_qp;
    _dgradp_qp_dv[_qp][1][_svar] = - d2pc_qp * _grads_qp[_qp][1];
    _dgradp_qp_dgradv[_qp][1][_svar] = - dpc_qp;
  }
}

void
PorousFlow2PhasePS::buildQpPPSS()
{
  _saturation_nodal[_qp][0] = 1.0 - _phase1_saturation_nodal[_node_number[_qp]];
  _saturation_nodal[_qp][1] = _phase1_saturation_nodal[_node_number[_qp]];
  _saturation_qp[_qp][0] = 1.0 - _phase1_saturation_qp[_qp];
  _saturation_qp[_qp][1] = _phase1_saturation_qp[_qp];
  _grads_qp[_qp][0] = -_phase1_grads_qp[_qp];
  _grads_qp[_qp][1] = _phase1_grads_qp[_qp];

  const Real pc_nodal = capillaryPressure(_phase1_saturation_nodal[_node_number[_qp]]);
  const Real pc_qp = capillaryPressure(_phase1_saturation_qp[_qp]);
  const Real dpc_qp = dCapillaryPressure_dS(_phase1_saturation_qp[_qp]);

  _porepressure_nodal[_qp][0] = _phase0_porepressure_nodal[_node_number[_qp]];
  _porepressure_nodal[_qp][1] = _phase0_porepressure_nodal[_node_number[_qp]] - pc_nodal;
  _porepressure_qp[_qp][0] = _phase0_porepressure_qp[_qp];
  _porepressure_qp[_qp][1] = _phase0_porepressure_qp[_qp] - pc_qp;
  _gradp_qp[_qp][0] = _phase0_gradp_qp[_qp];
  _gradp_qp[_qp][1] = _phase0_gradp_qp[_qp] - dpc_qp * _grads_qp[_qp][1];
}

Real
PorousFlow2PhasePS::capillaryPressure(Real /* saturation */) const
{
  return _pc;
}

Real
PorousFlow2PhasePS::dCapillaryPressure_dS(Real /* saturation */) const
{
  return 0.0;
}

Real
PorousFlow2PhasePS::d2CapillaryPressure_dS2(Real /* saturation */) const
{
  return 0.0;
}
