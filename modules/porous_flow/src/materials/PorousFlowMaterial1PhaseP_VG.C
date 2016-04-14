/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterial1PhaseP_VG.h"



template<>
InputParameters validParams<PorousFlowMaterial1PhaseP_VG>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredCoupledVar("porepressure", "Variable that represents the porepressure of the single phase");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addRequiredRangeCheckedParam<Real>("al", "al > 0", "van-Genuchten alpha parameter.  Must be positive.  effectiveSaturation = (1 + (-al*c)^(1/(1-m)))^(-m)");
  params.addRequiredRangeCheckedParam<Real>("m", "m > 0 & m < 1", "van-Genuchten m parameter.  Must be between 0 and 1, and optimally should be set to >0.5   EffectiveSaturation = (1 + (-al*p)^(1/(1-m)))^(-m)");
  params.addClassDescription("This Material is used for the single-phase situation where porepressure is the primary variable.  calculates the 1 porepressure and the 1 saturation in a 1-phase isothermal situation, and derivatives of these with respect to the PorousFlowVariables.  van-Genuchten capillarity is assumed");
  return params;
}

PorousFlowMaterial1PhaseP_VG::PorousFlowMaterial1PhaseP_VG(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _num_ph(1),
    _al(getParam<Real>("al")),
    _m(getParam<Real>("m")),


    _porepressure_var(coupledNodalValue("porepressure")),
    _qp_porepressure_var(coupledValue("porepressure")),
    _gradp_var(coupledGradient("porepressure")),
    _porepressure_varnum(coupled("porepressure")),

    _porflow_name_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),

    _porepressure(declareProperty<std::vector<Real> >("PorousFlow_porepressure")),
    _porepressure_old(declarePropertyOld<std::vector<Real> >("PorousFlow_porepressure")),
    _porepressure_qp(declareProperty<std::vector<Real> >("PorousFlow_porepressure_qp")),
    _gradp(declareProperty<std::vector<RealGradient> >("PorousFlow_grad_porepressure")),
    _dporepressure_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_dvar")),
    _dporepressure_qp_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_qp_dvar")),
    _dgradp_dgradv(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_grad_porepressure_dgradvar")),

    _saturation(declareProperty<std::vector<Real> >("PorousFlow_saturation")),
    _saturation_old(declarePropertyOld<std::vector<Real> >("PorousFlow_saturation")),
    _saturation_qp(declareProperty<std::vector<Real> >("PorousFlow_saturation_qp")),
    _grads(declareProperty<std::vector<RealGradient> >("PorousFlow_grad_saturation")),
    _dsaturation_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_dvar")),
    _dsaturation_qp_dvar(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_qp_dvar")),
    _dgrads_dgradv(declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_grad_saturation_dgradvar")),
    _dgrads_dv(declareProperty<std::vector<std::vector<RealGradient> > >("dPorousFlow_grad_saturation_dv"))
{
  if (_porflow_name_UO.num_phases() != 1)
    mooseError("The Dictator proclaims that the number of phases is " << _porflow_name_UO.num_phases() << " whereas PorousFlowMaterial1PhaseP_VG can only be used for 1-phase simulations.  Be aware that the Dictator has noted your mistake.");
}

void
PorousFlowMaterial1PhaseP_VG::initQpStatefulProperties()
{
  _porepressure[_qp].resize(_num_ph);
  _porepressure_qp[_qp].resize(_num_ph);
  _porepressure_old[_qp].resize(_num_ph);
  _gradp[_qp].resize(_num_ph);
  _dporepressure_dvar[_qp].resize(_num_ph);
  _dporepressure_qp_dvar[_qp].resize(_num_ph);
  _dgradp_dgradv[_qp].resize(_num_ph);

  _saturation[_qp].resize(_num_ph);
  _saturation_qp[_qp].resize(_num_ph);
  _saturation_old[_qp].resize(_num_ph);
  _grads[_qp].resize(_num_ph);
  _dsaturation_dvar[_qp].resize(_num_ph);
  _dsaturation_qp_dvar[_qp].resize(_num_ph);
  _dgrads_dgradv[_qp].resize(_num_ph);
  _dgrads_dv[_qp].resize(_num_ph);

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
PorousFlowMaterial1PhaseP_VG::computeQpProperties()
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
  /*if (_t_step == 1)
    for (unsigned ph = 0; ph < _num_ph; ++ph)
    {
      _porepressure_old[_qp][ph] = _porepressure[_qp][ph];
      _saturation_old[_qp][ph] = _saturation[_qp][ph];
    }
  */

  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_ph; ++phase)
  {
    _dporepressure_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dporepressure_qp_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dgradp_dgradv[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
  }

  // _porepressure is only dependent on _porepressure, and its derivative is 1
  if (!(_porflow_name_UO.not_porflow_var(_porepressure_varnum)))
  {
    // _porepressure is a porflow variable
    _dporepressure_dvar[_qp][0][_porflow_name_UO.porflow_var_num(_porepressure_varnum)] = 1;
    _dporepressure_qp_dvar[_qp][0][_porflow_name_UO.porflow_var_num(_porepressure_varnum)] = 1;
    _dgradp_dgradv[_qp][0][_porflow_name_UO.porflow_var_num(_porepressure_varnum)] = 1;
  }


  // prepare the derivative matrix with zeroes
  for (unsigned phase = 0; phase < _num_ph; ++phase)
  {
    _dsaturation_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dsaturation_qp_dvar[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dgrads_dgradv[_qp][phase].assign(_porflow_name_UO.num_v(), 0.0);
    _dgrads_dv[_qp][phase].assign(_porflow_name_UO.num_v(), RealGradient());
  }

  if (!(_porflow_name_UO.not_porflow_var(_porepressure_varnum)))
  {
    // _porepressure is a porflow variable
    _dsaturation_dvar[_qp][0][_porflow_name_UO.porflow_var_num(_porepressure_varnum)] = PorousFlowCapillaryVG::dseff(_porepressure_var[_qp], _al, _m);;
    _dsaturation_qp_dvar[_qp][0][_porflow_name_UO.porflow_var_num(_porepressure_varnum)] = PorousFlowCapillaryVG::dseff(_qp_porepressure_var[_qp], _al, _m);;
    _dgrads_dgradv[_qp][0][_porflow_name_UO.porflow_var_num(_porepressure_varnum)] = PorousFlowCapillaryVG::dseff(_qp_porepressure_var[_qp], _al, _m);
    _dgrads_dv[_qp][0][_porflow_name_UO.porflow_var_num(_porepressure_varnum)] = PorousFlowCapillaryVG::d2seff(_qp_porepressure_var[_qp], _al, _m)*_gradp_var[_qp];
  }
}

void
PorousFlowMaterial1PhaseP_VG::buildQpPPSS()
{
  _porepressure[_qp][0] = _porepressure_var[_qp];
  _porepressure_qp[_qp][0] = _qp_porepressure_var[_qp];
  _gradp[_qp][0] = _gradp_var[_qp];

  _saturation[_qp][0] = PorousFlowCapillaryVG::seff(_porepressure_var[_qp], _al, _m);
  _saturation_qp[_qp][0] = PorousFlowCapillaryVG::seff(_qp_porepressure_var[_qp], _al, _m);
  _grads[_qp][0] = PorousFlowCapillaryVG::dseff(_qp_porepressure_var[_qp], _al, _m)*_gradp_var[_qp];
}
