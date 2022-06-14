//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlow2PhasePP.h"
#include "PorousFlowCapillaryPressure.h"

registerMooseObject("PorousFlowApp", PorousFlow2PhasePP);

InputParameters
PorousFlow2PhasePP::validParams()
{
  InputParameters params = PorousFlowVariableBase::validParams();
  params.addRequiredCoupledVar("phase0_porepressure",
                               "Variable that is the porepressure of phase "
                               "0 (eg, the water phase).  It will be <= "
                               "phase1_porepressure.");
  params.addRequiredCoupledVar("phase1_porepressure",
                               "Variable that is the porepressure of phase 1 (eg, the gas phase)");
  params.addRequiredParam<UserObjectName>("capillary_pressure",
                                          "Name of the UserObject defining the capillary pressure");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations "
                             "in a 2-phase situation, and derivatives of these with "
                             "respect to the PorousFlowVariables");
  return params;
}

PorousFlow2PhasePP::PorousFlow2PhasePP(const InputParameters & parameters)
  : PorousFlowVariableBase(parameters),

    _phase0_porepressure(_nodal_material ? coupledDofValues("phase0_porepressure")
                                         : coupledValue("phase0_porepressure")),
    _phase0_gradp_qp(coupledGradient("phase0_porepressure")),
    _phase0_porepressure_varnum(coupled("phase0_porepressure")),
    _p0var(_dictator.isPorousFlowVariable(_phase0_porepressure_varnum)
               ? _dictator.porousFlowVariableNum(_phase0_porepressure_varnum)
               : 0),

    _phase1_porepressure(_nodal_material ? coupledDofValues("phase1_porepressure")
                                         : coupledValue("phase1_porepressure")),
    _phase1_gradp_qp(coupledGradient("phase1_porepressure")),
    _phase1_porepressure_varnum(coupled("phase1_porepressure")),
    _p1var(_dictator.isPorousFlowVariable(_phase1_porepressure_varnum)
               ? _dictator.porousFlowVariableNum(_phase1_porepressure_varnum)
               : 0),
    _pc_uo(getUserObject<PorousFlowCapillaryPressure>("capillary_pressure"))
{
  if (_num_phases != 2)
    mooseError("The Dictator announces that the number of phases is ",
               _dictator.numPhases(),
               " whereas PorousFlow2PhasePP can only be used for 2-phase simulation.  When you "
               "have an efficient government, you have a dictatorship.");
}

void
PorousFlow2PhasePP::initQpStatefulProperties()
{
  PorousFlowVariableBase::initQpStatefulProperties();
  buildQpPPSS();
}

void
PorousFlow2PhasePP::computeQpProperties()
{
  // size stuff correctly and prepare the derivative matrices with zeroes
  PorousFlowVariableBase::computeQpProperties();

  const Real pc = buildQpPPSS();
  const Real ds = _pc_uo.dSaturation(pc); // dS/d(pc)

  if (!_nodal_material)
  {
    (*_gradp_qp)[_qp][0] = _phase0_gradp_qp[_qp];
    (*_gradp_qp)[_qp][1] = _phase1_gradp_qp[_qp];
    (*_grads_qp)[_qp][0] = ds * ((*_gradp_qp)[_qp][0] - (*_gradp_qp)[_qp][1]);
    (*_grads_qp)[_qp][1] = -(*_grads_qp)[_qp][0];
  }

  // the derivatives of porepressure with respect to porepressure
  // remain fixed (at unity) throughout the simulation
  if (_dictator.isPorousFlowVariable(_phase0_porepressure_varnum))
  {
    (*_dporepressure_dvar)[_qp][0][_p0var] = 1.0;
    if (!_nodal_material)
      (*_dgradp_qp_dgradv)[_qp][0][_p0var] = 1.0;
  }
  if (_dictator.isPorousFlowVariable(_phase1_porepressure_varnum))
  {
    (*_dporepressure_dvar)[_qp][1][_p1var] = 1.0;
    if (!_nodal_material)
      (*_dgradp_qp_dgradv)[_qp][1][_p1var] = 1.0;
  }

  if (_dictator.isPorousFlowVariable(_phase0_porepressure_varnum))
  {
    (*_dsaturation_dvar)[_qp][0][_p0var] = ds;
    (*_dsaturation_dvar)[_qp][1][_p0var] = -ds;
  }
  if (_dictator.isPorousFlowVariable(_phase1_porepressure_varnum))
  {
    (*_dsaturation_dvar)[_qp][0][_p1var] = -ds;
    (*_dsaturation_dvar)[_qp][1][_p1var] = ds;
  }

  if (!_nodal_material)
  {
    const Real d2s_qp = _pc_uo.d2Saturation(pc); // d^2(S_qp)/d(pc_qp)^2
    if (_dictator.isPorousFlowVariable(_phase0_porepressure_varnum))
    {
      (*_dgrads_qp_dgradv)[_qp][0][_p0var] = ds;
      (*_dgrads_qp_dv)[_qp][0][_p0var] = d2s_qp * (_phase0_gradp_qp[_qp] - _phase1_gradp_qp[_qp]);
      (*_dgrads_qp_dgradv)[_qp][1][_p0var] = -ds;
      (*_dgrads_qp_dv)[_qp][1][_p0var] = -d2s_qp * (_phase0_gradp_qp[_qp] - _phase1_gradp_qp[_qp]);
    }
    if (_dictator.isPorousFlowVariable(_phase1_porepressure_varnum))
    {
      (*_dgrads_qp_dgradv)[_qp][0][_p1var] = -ds;
      (*_dgrads_qp_dv)[_qp][0][_p1var] = -d2s_qp * (_phase0_gradp_qp[_qp] - _phase1_gradp_qp[_qp]);
      (*_dgrads_qp_dgradv)[_qp][1][_p1var] = ds;
      (*_dgrads_qp_dv)[_qp][1][_p1var] = d2s_qp * (_phase0_gradp_qp[_qp] - _phase1_gradp_qp[_qp]);
    }
  }
}

Real
PorousFlow2PhasePP::buildQpPPSS()
{
  _porepressure[_qp][0] = _phase0_porepressure[_qp];
  _porepressure[_qp][1] = _phase1_porepressure[_qp];
  const Real pc = _phase0_porepressure[_qp] - _phase1_porepressure[_qp]; // this is <= 0
  const Real sat = _pc_uo.saturation(pc);
  _saturation[_qp][0] = sat;
  _saturation[_qp][1] = 1.0 - sat;
  return pc;
}
