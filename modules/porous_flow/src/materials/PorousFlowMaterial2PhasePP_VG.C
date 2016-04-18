/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterial2PhasePP_VG.h"



template<>
InputParameters validParams<PorousFlowMaterial2PhasePP_VG>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredCoupledVar("phase0_porepressure", "Variable that is the porepressure of phase 0 (eg, the water phase).  It will be <= phase1_porepressure.");
  params.addRequiredCoupledVar("phase1_porepressure", "Variable that is the porepressure of phase 1 (eg, the gas phase)");
  params.addCoupledVar("phase0_temperature", 20.0, "Fluid temperature of phase 0");
  params.addRequiredRangeCheckedParam<Real>("al", "al > 0", "van-Genuchten alpha parameter.  Must be positive.  effectiveSaturation = (1 + (-al*P)^(1/(1-m)))^(-m), where P = phase0_porepressure - phase1_porepressure <= 0");
  params.addRequiredRangeCheckedParam<Real>("m", "m > 0 & m < 1", "van-Genuchten m parameter.  Must be between 0 and 1, and optimally should be set to >0.5   EffectiveSaturation = (1 + (-al*p)^(1/(1-m)))^(-m)");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations in a 2-phase isothermal situation, and derivatives of these with respect to the PorousFlowVariables.  A van-Genuchten capillary suction function is assumed");
  return params;
}

PorousFlowMaterial2PhasePP_VG::PorousFlowMaterial2PhasePP_VG(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _num_ph(2),
    _al(getParam<Real>("al")),
    _m(getParam<Real>("m")),

    _phase0_porepressure(coupledNodalValue("phase0_porepressure")),
    _phase0_qp_porepressure(coupledValue("phase0_porepressure")),
    _phase0_gradp(coupledGradient("phase0_porepressure")),
    _phase0_porepressure_varnum(coupled("phase0_porepressure")),

    _phase1_porepressure(coupledNodalValue("phase1_porepressure")),
    _phase1_qp_porepressure(coupledValue("phase1_porepressure")),
    _phase1_gradp(coupledGradient("phase1_porepressure")),
    _phase1_porepressure_varnum(coupled("phase1_porepressure")),

    _phase0_temperature(coupledNodalValue("phase0_temperature")),
    _phase0_temperature_qp(coupledValue("phase0_temperature")),
    _phase0_temperature_varnum(coupled("phase0_temperature")),

    _porflow_name_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),

    _porepressure(declareProperty<std::vector<Real> >("PorousFlow_porepressure")),
    _porepressure_old(declarePropertyOld<std::vector<Real> >("PorousFlow_porepressure")),
    _porepressure_qp(declareProperty<std::vector<Real> >("PorousFlow_porepressure_qp")),
    _gradp(declareProperty<std::vector<RealGradient> >("PorousFlow_grad_porepressure")),
    _dporepressure_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_dvar")),
    _dporepressure_qp_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_qp_dvar")),
    _dgradp_dgradv(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_grad_porepressure_dgradvar")),
    _dgradp_dv(declareProperty<std::vector<std::vector<RealGradient> > >("dPorousFlow_grad_porepressure_dvar")),

    _saturation(declareProperty<std::vector<Real> >("PorousFlow_saturation")),
    _saturation_old(declarePropertyOld<std::vector<Real> >("PorousFlow_saturation")),
    _saturation_qp(declareProperty<std::vector<Real> >("PorousFlow_saturation_qp")),
    _grads(declareProperty<std::vector<RealGradient> >("PorousFlow_grad_saturation")),
    _dsaturation_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_dvar")),
    _dsaturation_qp_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_qp_dvar")),
    _dgrads_dgradv(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_grad_saturation_dgradvar")),
    _dgrads_dv(declareProperty<std::vector<std::vector<RealGradient> > >("dPorousFlow_grad_saturation_dvar")),

    _temperature(declareProperty<std::vector<Real> >("PorousFlow_temperature")),
    _temperature_qp(declareProperty<std::vector<Real> >("PorousFlow_temperature_qp")),
    _dtemperature_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_temperature_dvar")),
    _dtemperature_qp_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_temperature_qp_dvar"))

{
  if (_porflow_name_UO.num_phases() != _num_ph)
    mooseError("The Dictator announces that the number of phases is " << _porflow_name_UO.num_phases() << " whereas PorousFlowMaterial2PhasePP_VG can only be used for 2-phase simulation.  When you have an efficient government, you have a dictatorship.");
}

void
PorousFlowMaterial2PhasePP_VG::initQpStatefulProperties()
{
  _porepressure[_qp].resize(_num_ph);
  _porepressure_qp[_qp].resize(_num_ph);
  _porepressure_old[_qp].resize(_num_ph);
  _gradp[_qp].resize(_num_ph);
  _dporepressure_dvar[_qp].resize(_num_ph);
  _dporepressure_qp_dvar[_qp].resize(_num_ph);
  _dgradp_dgradv[_qp].resize(_num_ph);
  _dgradp_dv[_qp].resize(_num_ph);

  _saturation[_qp].resize(_num_ph);
  _saturation_qp[_qp].resize(_num_ph);
  _saturation_old[_qp].resize(_num_ph);
  _grads[_qp].resize(_num_ph);
  _dsaturation_dvar[_qp].resize(_num_ph);
  _dsaturation_qp_dvar[_qp].resize(_num_ph);
  _dgrads_dgradv[_qp].resize(_num_ph);
  _dgrads_dv[_qp].resize(_num_ph);

  _temperature[_qp].resize(_num_ph);
  _dtemperature_dvar[_qp].resize(_num_ph);
  _dtemperature_qp_dvar[_qp].resize(_num_ph);

  buildQpPPSS();

  /*
   * the derivatives of porepressure with respect to porepressure
   * remain fixed (at unity) throughout the simulation
   */
  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_ph; ++phase)
  {
    _dporepressure_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dporepressure_qp_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dgradp_dgradv[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dgradp_dv[_qp][phase].assign(_porflow_name_UO.num_v(), RealGradient());
  }

  if (!(_porflow_name_UO.not_porflow_var(_phase0_porepressure_varnum)))
  {
    // _phase0_porepressure is a porflow variable
    _dporepressure_dvar[_qp][0][_porflow_name_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1;
    _dporepressure_qp_dvar[_qp][0][_porflow_name_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1;
    _dgradp_dgradv[_qp][0][_porflow_name_UO.porflow_var_num(_phase0_porepressure_varnum)] = 1;
  }
  if (!(_porflow_name_UO.not_porflow_var(_phase1_porepressure_varnum)))
  {
    // _phase1_porepressure is a porflow variable
    _dporepressure_dvar[_qp][1][_porflow_name_UO.porflow_var_num(_phase1_porepressure_varnum)] = 1;
    _dporepressure_qp_dvar[_qp][1][_porflow_name_UO.porflow_var_num(_phase1_porepressure_varnum)] = 1;
    _dgradp_dgradv[_qp][1][_porflow_name_UO.porflow_var_num(_phase1_porepressure_varnum)] = 1;
  }

  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_ph; ++phase)
  {
    _dtemperature_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dtemperature_qp_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
  }

  // _temperature is only dependent on _phase0_temperature, and its derivative is = 1
  if (!(_porflow_name_UO.not_porflow_var(_phase0_temperature_varnum)))
  {
    // _phase0_temperature is a porflow variable
    _dtemperature_dvar[_qp][0][_porflow_name_UO.porflow_var_num(_phase0_temperature_varnum)] = 1.0;
    _dtemperature_dvar[_qp][1][_porflow_name_UO.porflow_var_num(_phase0_temperature_varnum)] = 1.0;
    _dtemperature_qp_dvar[_qp][0][_porflow_name_UO.porflow_var_num(_phase0_temperature_varnum)] = 1.0;
    _dtemperature_qp_dvar[_qp][1][_porflow_name_UO.porflow_var_num(_phase0_temperature_varnum)] = 1.0;
  }
}

void
PorousFlowMaterial2PhasePP_VG::computeQpProperties()
{
  buildQpPPSS();

  /* The derivatives of saturation with respect to
   * porepressure depend on porepressure
   */

  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_ph; ++phase)
  {
    _dsaturation_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dsaturation_qp_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dgrads_dgradv[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dgrads_dv[_qp][phase].assign(_porflow_name_UO.num_v(), RealGradient());
  }

  const Real pc = _phase0_porepressure[_qp] - _phase1_porepressure[_qp]; // this is <= 0
  const Real dseff = PorousFlowCapillaryVG::dseff(pc, _al, _m); // d(seff)/d(pc)
  const Real pc_qp = _phase0_qp_porepressure[_qp] - _phase1_qp_porepressure[_qp]; // this is <= 0
  const Real dseff_qp = PorousFlowCapillaryVG::dseff(pc_qp, _al, _m); // d(seff_qp)/d(pc_qp)
  const Real d2seff_qp = PorousFlowCapillaryVG::d2seff(pc_qp, _al, _m); // d^2(seff_qp)/d(pc_qp)^2

  if (!(_porflow_name_UO.not_porflow_var(_phase0_porepressure_varnum)))
  {
    const unsigned int pvar = _porflow_name_UO.porflow_var_num(_phase0_porepressure_varnum);
    _dsaturation_dvar[_qp][0][pvar] = dseff;
    _dsaturation_qp_dvar[_qp][0][pvar] = dseff_qp;
    _dgrads_dgradv[_qp][0][pvar] = dseff_qp;
    _dgrads_dv[_qp][0][pvar] = d2seff_qp*(_phase0_gradp[_qp] - _phase1_gradp[_qp]);

    _dsaturation_dvar[_qp][1][pvar] = -dseff;
    _dsaturation_qp_dvar[_qp][1][pvar] = -dseff_qp;
    _dgrads_dgradv[_qp][1][pvar] = -dseff_qp;
    _dgrads_dv[_qp][1][pvar] = -d2seff_qp*(_phase0_gradp[_qp] - _phase1_gradp[_qp]);
  }

  if (!(_porflow_name_UO.not_porflow_var(_phase1_porepressure_varnum)))
  {
    const unsigned int pvar = _porflow_name_UO.porflow_var_num(_phase1_porepressure_varnum);
    _dsaturation_dvar[_qp][0][pvar] = -dseff;
    _dsaturation_qp_dvar[_qp][0][pvar] = -dseff_qp;
    _dgrads_dgradv[_qp][0][pvar] = -dseff_qp;
    _dgrads_dv[_qp][0][pvar] = -d2seff_qp*(_phase0_gradp[_qp] - _phase1_gradp[_qp]);

    _dsaturation_dvar[_qp][1][pvar] = dseff;
    _dsaturation_qp_dvar[_qp][1][pvar] = dseff_qp;
    _dgrads_dgradv[_qp][1][pvar] = dseff_qp;
    _dgrads_dv[_qp][1][pvar] = d2seff_qp*(_phase0_gradp[_qp] - _phase1_gradp[_qp]);
  }

}

void
PorousFlowMaterial2PhasePP_VG::buildQpPPSS()
{
  _porepressure[_qp][0] = _phase0_porepressure[_qp];
  _porepressure[_qp][1] = _phase1_porepressure[_qp];
  _porepressure_qp[_qp][0] = _phase0_qp_porepressure[_qp];
  _porepressure_qp[_qp][1] = _phase1_qp_porepressure[_qp];
  _gradp[_qp][0] = _phase0_gradp[_qp];
  _gradp[_qp][1] = _phase1_gradp[_qp];

  const Real pc = _phase0_porepressure[_qp] - _phase1_porepressure[_qp]; // this is <= 0
  const Real seff = PorousFlowCapillaryVG::seff(pc, _al, _m);

  const Real pc_qp = _phase0_qp_porepressure[_qp] - _phase1_qp_porepressure[_qp]; // this is <= 0
  const Real seff_qp = PorousFlowCapillaryVG::seff(pc_qp, _al, _m);
  const Real dseff_qp = PorousFlowCapillaryVG::dseff(pc_qp, _al, _m); // d(seff_qp)/d(pc_qp)

  _saturation[_qp][0] = seff;
  _saturation[_qp][1] = 1.0 - seff;
  _saturation_qp[_qp][0] = seff_qp;
  _saturation_qp[_qp][1] = 1.0 - seff_qp;
  _grads[_qp][0] = dseff_qp*(_gradp[_qp][0] - _gradp[_qp][1]);
  _grads[_qp][1] = -_grads[_qp][0];

  /// Temperature is the same in each phase presently
  _temperature[_qp][0] = _phase0_temperature[_qp];
  _temperature[_qp][1] = _phase0_temperature[_qp];
  _temperature_qp[_qp][0] = _phase0_temperature_qp[_qp];
  _temperature_qp[_qp][1] = _phase0_temperature_qp[_qp];
}
