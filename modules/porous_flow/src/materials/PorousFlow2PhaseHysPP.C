//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlow2PhaseHysPP.h"

registerMooseObject("PorousFlowApp", PorousFlow2PhaseHysPP);

InputParameters
PorousFlow2PhaseHysPP::validParams()
{
  InputParameters params = PorousFlowHystereticCapillaryPressure::validParams();
  params.addRequiredCoupledVar("phase0_porepressure",
                               "Variable that is the porepressure of phase 0.  This is assumed to "
                               "be the liquid phase.  It will be <= phase1_porepressure.");
  params.addRequiredCoupledVar("phase1_porepressure",
                               "Variable that is the porepressure of phase 1 (the gas phase)");
  params.addParam<unsigned>(
      "liquid_phase", 0, "Phase number of the liquid phase (more precisely, the phase of phase0)");
  params.addClassDescription(
      "This Material is used for 2-phase situations.  It calculates the 2 saturations given the 2 "
      "porepressures, assuming the capillary pressure is hysteretic.  Derivatives of these "
      "quantities are also computed");
  return params;
}

PorousFlow2PhaseHysPP::PorousFlow2PhaseHysPP(const InputParameters & parameters)
  : PorousFlowHystereticCapillaryPressure(parameters),
    _pc(_nodal_material ? declareProperty<Real>("PorousFlow_hysteretic_capillary_pressure_nodal")
                        : declareProperty<Real>("PorousFlow_hysteretic_capillary_pressure_qp")),
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
               : 0)
{
  if (_num_phases != 2)
    mooseError("The Dictator announces that the number of phases is ",
               _dictator.numPhases(),
               " whereas PorousFlow2PhaseHysPP can only be used for 2-phase simulation.  When you "
               "have efficient government, you have dictatorship.");
}

void
PorousFlow2PhaseHysPP::initQpStatefulProperties()
{
  PorousFlowHystereticCapillaryPressure::initQpStatefulProperties();
  buildQpPPSS();
}

void
PorousFlow2PhaseHysPP::computeQpProperties()
{
  // size stuff correctly and prepare the derivative matrices with zeroes
  PorousFlowHystereticCapillaryPressure::computeQpProperties();

  buildQpPPSS();
  const Real pc = _pc[_qp];                // >= 0
  const Real ds = dliquidSaturationQp(pc); // dS/d(pc)

  if (!_nodal_material)
  {
    (*_gradp_qp)[_qp][0] = _phase0_gradp_qp[_qp];
    (*_gradp_qp)[_qp][1] = _phase1_gradp_qp[_qp];
    (*_grads_qp)[_qp][0] = ds * ((*_gradp_qp)[_qp][1] - (*_gradp_qp)[_qp][0]);
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
    (*_dsaturation_dvar)[_qp][0][_p0var] = -ds;
    (*_dsaturation_dvar)[_qp][1][_p0var] = ds;
  }
  if (_dictator.isPorousFlowVariable(_phase1_porepressure_varnum))
  {
    (*_dsaturation_dvar)[_qp][0][_p1var] = ds;
    (*_dsaturation_dvar)[_qp][1][_p1var] = -ds;
  }

  _pc[_qp] = _phase1_porepressure[_qp] - _phase0_porepressure[_qp]; // this is >= 0
  if (!_nodal_material)
  {
    const Real d2s_qp = d2liquidSaturationQp(pc); // d^2(S_qp)/d(pc_qp)^2
    if (_dictator.isPorousFlowVariable(_phase0_porepressure_varnum))
    {
      (*_dgrads_qp_dgradv)[_qp][0][_p0var] = -ds;
      (*_dgrads_qp_dv)[_qp][0][_p0var] = -d2s_qp * (_phase1_gradp_qp[_qp] - _phase0_gradp_qp[_qp]);
      (*_dgrads_qp_dgradv)[_qp][1][_p0var] = ds;
      (*_dgrads_qp_dv)[_qp][1][_p0var] = d2s_qp * (_phase1_gradp_qp[_qp] - _phase0_gradp_qp[_qp]);
    }
    if (_dictator.isPorousFlowVariable(_phase1_porepressure_varnum))
    {
      (*_dgrads_qp_dgradv)[_qp][0][_p1var] = ds;
      (*_dgrads_qp_dv)[_qp][0][_p1var] = d2s_qp * (_phase1_gradp_qp[_qp] - _phase0_gradp_qp[_qp]);
      (*_dgrads_qp_dgradv)[_qp][1][_p1var] = -ds;
      (*_dgrads_qp_dv)[_qp][1][_p1var] = -d2s_qp * (_phase1_gradp_qp[_qp] - _phase0_gradp_qp[_qp]);
    }
  }
}

void
PorousFlow2PhaseHysPP::buildQpPPSS()
{
  _porepressure[_qp][0] = _phase0_porepressure[_qp];
  _porepressure[_qp][1] = _phase1_porepressure[_qp];
  _pc[_qp] = _phase1_porepressure[_qp] - _phase0_porepressure[_qp]; // this is >= 0
  const Real sat = liquidSaturationQp(_pc[_qp]);
  _saturation[_qp][0] = sat;
  _saturation[_qp][1] = 1.0 - sat;
}
