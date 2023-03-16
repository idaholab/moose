//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlow2PhasePS.h"
#include "PorousFlowCapillaryPressure.h"

registerMooseObject("PorousFlowApp", PorousFlow2PhasePS);

InputParameters
PorousFlow2PhasePS::validParams()
{
  InputParameters params = PorousFlowVariableBase::validParams();
  params.addRequiredCoupledVar("phase0_porepressure",
                               "Variable that is the porepressure of phase 0 (the liquid phase)");
  params.addRequiredCoupledVar("phase1_saturation",
                               "Variable that is the saturation of phase 1 (the gas phase)");
  params.addRequiredParam<UserObjectName>("capillary_pressure",
                                          "Name of the UserObject defining the capillary pressure");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations "
                             "in a 2-phase situation, and derivatives of these with "
                             "respect to the PorousFlowVariables.");
  return params;
}

PorousFlow2PhasePS::PorousFlow2PhasePS(const InputParameters & parameters)
  : PorousFlowVariableBase(parameters),

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
              : 0),

    _pc_uo(getUserObject<PorousFlowCapillaryPressure>("capillary_pressure"))
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

  buildQpPPSS();
  const Real dpc = _pc_uo.dCapillaryPressure(1.0 - _phase1_saturation[_qp]);

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
      const Real d2pc_qp = _pc_uo.d2CapillaryPressure(1.0 - _phase1_saturation[_qp]);
      (*_dgradp_qp_dv)[_qp][1][_svar] = d2pc_qp * (*_grads_qp)[_qp][1];
      (*_dgradp_qp_dgradv)[_qp][1][_svar] = -dpc;
    }
  }
}

void
PorousFlow2PhasePS::buildQpPPSS()
{
  _saturation[_qp][0] = 1.0 - _phase1_saturation[_qp];
  _saturation[_qp][1] = _phase1_saturation[_qp];
  const Real pc = _pc_uo.capillaryPressure(1.0 - _phase1_saturation[_qp]);
  _porepressure[_qp][0] = _phase0_porepressure[_qp];
  _porepressure[_qp][1] = _phase0_porepressure[_qp] + pc;
}
