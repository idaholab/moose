/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlow2PhasePP.h"

template <>
InputParameters
validParams<PorousFlow2PhasePP>()
{
  InputParameters params = validParams<PorousFlowVariableBase>();

  params.addRequiredCoupledVar("phase0_porepressure",
                               "Variable that is the porepressure of phase "
                               "0 (eg, the water phase).  It will be <= "
                               "phase1_porepressure.");
  params.addRequiredCoupledVar("phase1_porepressure",
                               "Variable that is the porepressure of phase 1 (eg, the gas phase)");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations "
                             "in a 2-phase isothermal situation, and derivatives of these with "
                             "respect to the PorousFlowVariables");
  return params;
}

PorousFlow2PhasePP::PorousFlow2PhasePP(const InputParameters & parameters)
  : PorousFlowVariableBase(parameters),

    _phase0_porepressure(_nodal_material ? coupledNodalValue("phase0_porepressure")
                                         : coupledValue("phase0_porepressure")),
    _phase0_gradp_qp(coupledGradient("phase0_porepressure")),
    _phase0_porepressure_varnum(coupled("phase0_porepressure")),
    _p0var(_dictator.isPorousFlowVariable(_phase0_porepressure_varnum)
               ? _dictator.porousFlowVariableNum(_phase0_porepressure_varnum)
               : 0),

    _phase1_porepressure(_nodal_material ? coupledNodalValue("phase1_porepressure")
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
  const Real dseff = dEffectiveSaturation_dP(pc); // d(seff)/d(pc)

  if (!_nodal_material)
  {
    (*_gradp_qp)[_qp][0] = _phase0_gradp_qp[_qp];
    (*_gradp_qp)[_qp][1] = _phase1_gradp_qp[_qp];
    (*_grads_qp)[_qp][0] = dseff * ((*_gradp_qp)[_qp][0] - (*_gradp_qp)[_qp][1]);
    (*_grads_qp)[_qp][1] = -(*_grads_qp)[_qp][0];
  }

  // the derivatives of porepressure with respect to porepressure
  // remain fixed (at unity) throughout the simulation
  if (_dictator.isPorousFlowVariable(_phase0_porepressure_varnum))
  {
    _dporepressure_dvar[_qp][0][_p0var] = 1.0;
    if (!_nodal_material)
      (*_dgradp_qp_dgradv)[_qp][0][_p0var] = 1.0;
  }
  if (_dictator.isPorousFlowVariable(_phase1_porepressure_varnum))
  {
    _dporepressure_dvar[_qp][1][_p1var] = 1.0;
    if (!_nodal_material)
      (*_dgradp_qp_dgradv)[_qp][1][_p1var] = 1.0;
  }

  if (_dictator.isPorousFlowVariable(_phase0_porepressure_varnum))
  {
    _dsaturation_dvar[_qp][0][_p0var] = dseff;
    _dsaturation_dvar[_qp][1][_p0var] = -dseff;
  }
  if (_dictator.isPorousFlowVariable(_phase1_porepressure_varnum))
  {
    _dsaturation_dvar[_qp][0][_p1var] = -dseff;
    _dsaturation_dvar[_qp][1][_p1var] = dseff;
  }

  if (!_nodal_material)
  {
    const Real d2seff_qp = d2EffectiveSaturation_dP2(pc); // d^2(seff_qp)/d(pc_qp)^2
    if (_dictator.isPorousFlowVariable(_phase0_porepressure_varnum))
    {
      (*_dgrads_qp_dgradv)[_qp][0][_p0var] = dseff;
      (*_dgrads_qp_dv)[_qp][0][_p0var] =
          d2seff_qp * (_phase0_gradp_qp[_qp] - _phase1_gradp_qp[_qp]);
      (*_dgrads_qp_dgradv)[_qp][1][_p0var] = -dseff;
      (*_dgrads_qp_dv)[_qp][1][_p0var] =
          -d2seff_qp * (_phase0_gradp_qp[_qp] - _phase1_gradp_qp[_qp]);
    }
    if (_dictator.isPorousFlowVariable(_phase1_porepressure_varnum))
    {
      (*_dgrads_qp_dgradv)[_qp][0][_p1var] = -dseff;
      (*_dgrads_qp_dv)[_qp][0][_p1var] =
          -d2seff_qp * (_phase0_gradp_qp[_qp] - _phase1_gradp_qp[_qp]);
      (*_dgrads_qp_dgradv)[_qp][1][_p1var] = dseff;
      (*_dgrads_qp_dv)[_qp][1][_p1var] =
          d2seff_qp * (_phase0_gradp_qp[_qp] - _phase1_gradp_qp[_qp]);
    }
  }
}

Real
PorousFlow2PhasePP::buildQpPPSS()
{
  _porepressure[_qp][0] = _phase0_porepressure[_qp];
  _porepressure[_qp][1] = _phase1_porepressure[_qp];
  const Real pc = _phase0_porepressure[_qp] - _phase1_porepressure[_qp]; // this is <= 0
  const Real seff = effectiveSaturation(pc);
  _saturation[_qp][0] = seff;
  _saturation[_qp][1] = 1.0 - seff;
  return pc;
}

Real PorousFlow2PhasePP::effectiveSaturation(Real /* pressure */) const { return 1.0; }

Real PorousFlow2PhasePP::dEffectiveSaturation_dP(Real /* pressure */) const { return 0.0; }

Real PorousFlow2PhasePP::d2EffectiveSaturation_dP2(Real /* pressure */) const { return 0.0; }
