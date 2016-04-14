/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorFlowMaterial2PhasePS.h"



template<>
InputParameters validParams<PorFlowMaterial2PhasePS>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredCoupledVar("phase0_porepressure", "Variable that is the porepressure of phase 0 (eg, the gas phase)");
  params.addRequiredCoupledVar("phase1_saturation", "Variable that is the saturation of phase 1 (eg, the water phase)");
  params.addRequiredParam<UserObjectName>("PorFlowVarNames_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations in a 2-phase isothermal situation, and derivatives of these with respect to the PorFlowVariables.");
  return params;
}

PorFlowMaterial2PhasePS::PorFlowMaterial2PhasePS(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _phase0_porepressure(coupledNodalValue("phase0_porepressure")),
    _phase0_qp_porepressure(coupledValue("phase0_porepressure")),
    _phase0_gradp(coupledGradient("phase0_porepressure")),
    _phase0_porepressure_varnum(coupled("phase0_porepressure")),

    _phase1_saturation(coupledNodalValue("phase1_saturation")),
    _phase1_qp_saturation(coupledValue("phase1_saturation")),
    _phase1_grads(coupledGradient("phase1_saturation")),
    _phase1_saturation_varnum(coupled("phase1_saturation")),

    _porflow_name_UO(getUserObject<PorFlowVarNames>("PorFlowVarNames_UO")),

    _porepressure(declareProperty<std::vector<Real> >("PorFlow_porepressure")),
    _porepressure_old(declarePropertyOld<std::vector<Real> >("PorFlow_porepressure")),
    _porepressure_qp(declareProperty<std::vector<Real> >("PorFlow_porepressure_qp")),
    _gradp(declareProperty<std::vector<RealGradient> >("PorFlow_grad_porepressure")),
    _dporepressure_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorFlow_porepressure_dvar")),
    _dporepressure_qp_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorFlow_porepressure_qp_dvar")),
    _dgradp_dgradv(declareProperty<std::vector<std::vector<Real> > >("dPorFlow_grad_porepressure_dgradvar")),

    _saturation(declareProperty<std::vector<Real> >("PorFlow_saturation")),
    _saturation_old(declarePropertyOld<std::vector<Real> >("PorFlow_saturation")),
    _saturation_qp(declareProperty<std::vector<Real> >("PorFlow_saturation_qp")),
    _grads(declareProperty<std::vector<RealGradient> >("PorFlow_grad_saturation")),
    _dsaturation_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorFlow_saturation_dvar")),
    _dsaturation_qp_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorFlow_saturation_qp_dvar")),
    _dgrads_dgradv(declareProperty<std::vector<std::vector<Real> > >("dPorFlow_grad_saturation_dgradvar"))
{
}

void
PorFlowMaterial2PhasePS::initQpStatefulProperties()
{
  _porepressure[_qp].resize(2);
  _porepressure_qp[_qp].resize(2);
  _porepressure_old[_qp].resize(2);
  _gradp[_qp].resize(2);
  _dporepressure_dvar[_qp].resize(2);
  _dporepressure_qp_dvar[_qp].resize(2);
  _dgradp_dgradv[_qp].resize(2);

  _saturation[_qp].resize(2);
  _saturation_qp[_qp].resize(2);
  _saturation_old[_qp].resize(2);
  _grads[_qp].resize(2);
  _dsaturation_dvar[_qp].resize(2);
  _dsaturation_qp_dvar[_qp].resize(2);
  _dgrads_dgradv[_qp].resize(2);

  /*
   *  YAQI HACK !!
   *
   * I really just want to simply call
   * buildQpPPSS();
   * but i do it below in computeQpProperties
   */
}

void
PorFlowMaterial2PhasePS::computeQpProperties()
{
  buildQpPPSS();

  /*
   *  YAQI HACK !!
   *
   * I really just want to simply call
   * buildQpPPSS();
   * from initQpStatefulProperties, but the Variables
   * aren't initialised at that point so moose crashes
   */
  if (_t_step == 1)
    for (unsigned ph = 0; ph < 2; ++ph)
    {
      _porepressure_old[_qp][ph] = _porepressure[_qp][ph];
      _saturation_old[_qp][ph] = _saturation[_qp][ph];
    }

  /*
   * TODO: these derivatives could be put into the initQpStatefulProperties
   *       but only if i keep pc=constant, which is probably unphysical, so i'll
   *       keep the following computations here for modification with
   *       a physical pc later.
   */
  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < 2; ++phase)
  {
    _dporepressure_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dporepressure_qp_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dgradp_dgradv[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
  }

  // _porepressure is only dependent on _phase0_porepressure, and its derivative is 1
  if (!(_porflow_name_UO.not_porflow_var(_phase0_porepressure_varnum)))
  {
    // _phase0_porepressure is a porflow variable
    for (unsigned phase = 0; phase < 2; ++phase)
    {
      _dporepressure_dvar[_qp][phase][_porflow_name_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1;
      _dporepressure_qp_dvar[_qp][phase][_porflow_name_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1;
      _dgradp_dgradv[_qp][phase][_porflow_name_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1;
    }
  }


  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < 2; ++phase)
  {
    _dsaturation_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dsaturation_qp_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dgrads_dgradv[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
  }

  // _saturation is only dependent on _phase1_saturation, and its derivative is +/- 1
  if (!(_porflow_name_UO.not_porflow_var(_phase1_saturation_varnum)))
  {
    // _phase1_saturation is a porflow variable
    _dsaturation_dvar[_qp][0][_porflow_name_UO.porflow_var_num(_phase1_saturation_varnum)] = -1;
    _dsaturation_dvar[_qp][1][_porflow_name_UO.porflow_var_num(_phase1_saturation_varnum)] = 1;
    _dsaturation_qp_dvar[_qp][0][_porflow_name_UO.porflow_var_num(_phase1_saturation_varnum)] = -1;
    _dsaturation_qp_dvar[_qp][1][_porflow_name_UO.porflow_var_num(_phase1_saturation_varnum)] = 1;
    _dgrads_dgradv[_qp][0][_porflow_name_UO.porflow_var_num(_phase1_saturation_varnum)] = -1;
    _dgrads_dgradv[_qp][1][_porflow_name_UO.porflow_var_num(_phase1_saturation_varnum)] = 1;
  }
}

void
PorFlowMaterial2PhasePS::buildQpPPSS()
{
  const Real pc = 1.0;  // capillary suction function
  const Real pc_qp = 1.0;
  _porepressure[_qp][0] = _phase0_porepressure[_qp];
  _porepressure[_qp][1] = _phase0_porepressure[_qp] - pc;
  _porepressure_qp[_qp][0] = _phase0_qp_porepressure[_qp];
  _porepressure_qp[_qp][1] = _phase0_qp_porepressure[_qp] - pc_qp;
  _gradp[_qp][0] = _phase0_gradp[_qp];
  _gradp[_qp][1] = _phase0_gradp[_qp];

  _saturation[_qp][0] = 1.0 - _phase1_saturation[_qp];
  _saturation[_qp][1] = _phase1_saturation[_qp];
  _saturation_qp[_qp][0] = 1.0 - _phase1_qp_saturation[_qp];
  _saturation_qp[_qp][1] = _phase1_qp_saturation[_qp];
  _grads[_qp][0] = -_phase1_grads[_qp];
  _grads[_qp][1] = _phase1_grads[_qp];
}
