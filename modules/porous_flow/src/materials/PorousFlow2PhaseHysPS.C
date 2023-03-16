//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlow2PhaseHysPS.h"

registerMooseObject("PorousFlowApp", PorousFlow2PhaseHysPS);

InputParameters
PorousFlow2PhaseHysPS::validParams()
{
  InputParameters params = PorousFlowHystereticCapillaryPressure::validParams();
  params.addRequiredCoupledVar(
      "phase0_porepressure",
      "Variable that is the porepressure of phase 0, which is the liquid phase");
  params.addRequiredCoupledVar(
      "phase1_saturation", "Variable that is the saturation of phase 1, which is the gas phase");
  params.addClassDescription("This Material is used for 2-phase situations.  It calculates the 2 "
                             "saturations and 2 porepressures, assuming the capillary pressure is "
                             "hysteretic.  Derivatives of these quantities are also computed");
  return params;
}

PorousFlow2PhaseHysPS::PorousFlow2PhaseHysPS(const InputParameters & parameters)
  : PorousFlowHystereticCapillaryPressure(parameters),
    _pc(_nodal_material ? declareProperty<Real>("PorousFlow_hysteretic_capillary_pressure_nodal")
                        : declareProperty<Real>("PorousFlow_hysteretic_capillary_pressure_qp")),
    _phase0_porepressure(_nodal_material ? coupledDofValues("phase0_porepressure")
                                         : coupledValue("phase0_porepressure")),
    _phase0_gradp_qp(coupledGradient("phase0_porepressure")),
    _phase0_porepressure_varnum(coupled("phase0_porepressure")),
    _pvar(_dictator.isPorousFlowVariable(_phase0_porepressure_varnum)
              ? _dictator.porousFlowVariableNum(_phase0_porepressure_varnum)
              : 0),

    _phase1_saturation(_nodal_material ? coupledDofValues("phase1_saturation")
                                       : coupledValue("phase1_saturation")),
    _phase1_grads_qp(coupledGradient("phase1_saturation")),
    _phase1_saturation_varnum(coupled("phase1_saturation")),
    _svar(_dictator.isPorousFlowVariable(_phase1_saturation_varnum)
              ? _dictator.porousFlowVariableNum(_phase1_saturation_varnum)
              : 0)
{
  if (_num_phases != 2)
    mooseError("The Dictator announces that the number of phases is ",
               _dictator.numPhases(),
               " whereas PorousFlow2PhaseHysPS can only be used for 2-phase simulation.  The "
               "Dictator is always watching.");
}

void
PorousFlow2PhaseHysPS::initQpStatefulProperties()
{
  PorousFlowHystereticCapillaryPressure::initQpStatefulProperties();
  buildQpPPSS();
}

void
PorousFlow2PhaseHysPS::computeQpProperties()
{
  // size stuff correctly and prepare the derivative matrices with zeroes
  PorousFlowHystereticCapillaryPressure::computeQpProperties();

  buildQpPPSS();
  const Real dpc = dcapillaryPressureQp(1.0 - _phase1_saturation[_qp]); // d(Pc)/d(S0)

  if (!_nodal_material)
  {
    (*_grads_qp)[_qp][0] = -_phase1_grads_qp[_qp];
    (*_grads_qp)[_qp][1] = _phase1_grads_qp[_qp];
    (*_gradp_qp)[_qp][0] = _phase0_gradp_qp[_qp];
    (*_gradp_qp)[_qp][1] = _phase0_gradp_qp[_qp] - dpc * (*_grads_qp)[_qp][1];
  }

  // _porepressure depends on _phase0_porepressure, and its derivative is 1
  if (_dictator.isPorousFlowVariable(_phase0_porepressure_varnum))
  {
    // _phase0_porepressure is a PorousFlow variable
    for (unsigned phase = 0; phase < _num_phases; ++phase)
    {
      (*_dporepressure_dvar)[_qp][phase][_pvar] = 1.0;
      if (!_nodal_material)
        (*_dgradp_qp_dgradv)[_qp][phase][_pvar] = 1.0;
    }
  }

  // _saturation is only dependent on _phase1_saturation, and its derivative is +/- 1
  if (_dictator.isPorousFlowVariable(_phase1_saturation_varnum))
  {
    // _phase1_saturation is a PorousFlow variable
    // _phase1_porepressure depends on saturation through the capillary pressure function
    (*_dsaturation_dvar)[_qp][0][_svar] = -1.0;
    (*_dsaturation_dvar)[_qp][1][_svar] = 1.0;
    (*_dporepressure_dvar)[_qp][1][_svar] = -dpc;

    if (!_nodal_material)
    {
      (*_dgrads_qp_dgradv)[_qp][0][_svar] = -1.0;
      (*_dgrads_qp_dgradv)[_qp][1][_svar] = 1.0;
      const Real d2pc_qp = d2capillaryPressureQp(1.0 - _phase1_saturation[_qp]); // d^2(Pc)/dS0^2
      (*_dgradp_qp_dv)[_qp][1][_svar] = d2pc_qp * (*_grads_qp)[_qp][1];
      (*_dgradp_qp_dgradv)[_qp][1][_svar] = -dpc;
    }
  }
}

void
PorousFlow2PhaseHysPS::buildQpPPSS()
{
  _saturation[_qp][0] = 1.0 - _phase1_saturation[_qp];
  _saturation[_qp][1] = _phase1_saturation[_qp];
  _pc[_qp] = capillaryPressureQp(1.0 - _phase1_saturation[_qp]);
  _porepressure[_qp][0] = _phase0_porepressure[_qp];
  _porepressure[_qp][1] = _phase0_porepressure[_qp] + _pc[_qp];
}
