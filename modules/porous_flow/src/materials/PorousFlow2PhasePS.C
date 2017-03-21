/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlow2PhasePS.h"

template <>
InputParameters
validParams<PorousFlow2PhasePS>()
{
  InputParameters params = validParams<PorousFlowVariableBase>();
  params.addRequiredCoupledVar("phase0_porepressure",
                               "Variable that is the porepressure of phase 0 (eg, the gas phase)");
  params.addRequiredCoupledVar("phase1_saturation",
                               "Variable that is the saturation of phase 1 (eg, the water phase)");
  params.addRangeCheckedParam<Real>(
      "pc",
      0.0,
      "pc <= 0",
      "Constant capillary pressure (Pa). Default is 0. Note: capillary pressure must be negative");
  params.addRangeCheckedParam<Real>(
      "sat_lr",
      0.0,
      "sat_lr >= 0 & sat_lr <= 1",
      "Liquid residual saturation.  Must be between 0 and 1. Default is 0");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations "
                             "in a 2-phase isothermal situation, and derivatives of these with "
                             "respect to the PorousFlowVariables.");
  return params;
}

PorousFlow2PhasePS::PorousFlow2PhasePS(const InputParameters & parameters)
  : PorousFlowVariableBase(parameters),

    _phase0_porepressure(_nodal_material ? coupledNodalValue("phase0_porepressure")
                                         : coupledValue("phase0_porepressure")),
    _phase0_gradp_qp(coupledGradient("phase0_porepressure")),
    _phase0_porepressure_varnum(coupled("phase0_porepressure")),
    _pvar(_dictator.isPorousFlowVariable(_phase0_porepressure_varnum)
              ? _dictator.porousFlowVariableNum(_phase0_porepressure_varnum)
              : 0),

    _phase1_saturation(_nodal_material ? coupledNodalValue("phase1_saturation")
                                       : coupledValue("phase1_saturation")),
    _phase1_grads_qp(coupledGradient("phase1_saturation")),
    _phase1_saturation_varnum(coupled("phase1_saturation")),
    _svar(_dictator.isPorousFlowVariable(_phase1_saturation_varnum)
              ? _dictator.porousFlowVariableNum(_phase1_saturation_varnum)
              : 0),

    _pc(getParam<Real>("pc")),
    _sat_lr(getParam<Real>("sat_lr")),
    _dseff_ds(1.0 / (1.0 - _sat_lr))
{
  if (_dictator.numPhases() != 2)
    mooseError("The Dictator proclaims that the number of phases is ",
               _dictator.numPhases(),
               " whereas PorousFlow2PhasePS can only be used for 2-phase simulation.  Be aware "
               "that the Dictator has noted your mistake.");
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
  // size stuff correctly and prepare the derivative matrices with zeroes
  PorousFlowVariableBase::computeQpProperties();

  const Real seff = buildQpPPSS();
  const Real dpc = dCapillaryPressure_dS(seff) * _dseff_ds;

  if (!_nodal_material)
  {
    (*_grads_qp)[_qp][0] = -_phase1_grads_qp[_qp];
    (*_grads_qp)[_qp][1] = _phase1_grads_qp[_qp];
    (*_gradp_qp)[_qp][0] = _phase0_gradp_qp[_qp];
    (*_gradp_qp)[_qp][1] = _phase0_gradp_qp[_qp] + dpc * (*_grads_qp)[_qp][1];
  }

  // _porepressure depends on _phase0_porepressure, and its derivative is 1
  if (_dictator.isPorousFlowVariable(_phase0_porepressure_varnum))
  {
    // _phase0_porepressure is a PorousFlow variable
    for (unsigned phase = 0; phase < _num_phases; ++phase)
    {
      _dporepressure_dvar[_qp][phase][_pvar] = 1.0;
      if (!_nodal_material)
        (*_dgradp_qp_dgradv)[_qp][phase][_pvar] = 1.0;
    }
  }

  // _saturation is only dependent on _phase1_saturation, and its derivative is +/- 1
  if (_dictator.isPorousFlowVariable(_phase1_saturation_varnum))
  {
    // _phase1_saturation is a porflow variable
    // _phase1_porepressure depends on saturation through the capillary pressure function
    _dsaturation_dvar[_qp][0][_svar] = -1.0;
    _dsaturation_dvar[_qp][1][_svar] = 1.0;
    _dporepressure_dvar[_qp][1][_svar] = dpc;

    if (!_nodal_material)
    {
      (*_dgrads_qp_dgradv)[_qp][0][_svar] = -1.0;
      (*_dgrads_qp_dgradv)[_qp][1][_svar] = 1.0;
      const Real d2pc_qp = d2CapillaryPressure_dS2(seff) * _dseff_ds * _dseff_ds;
      (*_dgradp_qp_dv)[_qp][1][_svar] = d2pc_qp * (*_grads_qp)[_qp][1];
      (*_dgradp_qp_dgradv)[_qp][1][_svar] = dpc;
    }
  }
}

Real
PorousFlow2PhasePS::buildQpPPSS()
{
  _saturation[_qp][0] = 1.0 - _phase1_saturation[_qp];
  _saturation[_qp][1] = _phase1_saturation[_qp];
  const Real seff = effectiveSaturation(_phase1_saturation[_qp]);
  const Real pc = capillaryPressure(seff);
  _porepressure[_qp][0] = _phase0_porepressure[_qp];
  _porepressure[_qp][1] = _phase0_porepressure[_qp] + pc;
  return seff;
}

Real
PorousFlow2PhasePS::effectiveSaturation(Real saturation) const
{
  return (saturation - _sat_lr) / (1.0 - _sat_lr);
}

Real PorousFlow2PhasePS::capillaryPressure(Real /* saturation */) const { return _pc; }

Real PorousFlow2PhasePS::dCapillaryPressure_dS(Real /* saturation */) const { return 0.0; }

Real PorousFlow2PhasePS::d2CapillaryPressure_dS2(Real /* saturation */) const { return 0.0; }
