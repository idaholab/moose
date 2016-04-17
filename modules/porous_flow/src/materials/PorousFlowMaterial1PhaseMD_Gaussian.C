/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterial1PhaseMD_Gaussian.h"



template<>
InputParameters validParams<PorousFlowMaterial1PhaseMD_Gaussian>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredCoupledVar("mass_density", "Variable that represents log(mass-density) of the single phase");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addRequiredRangeCheckedParam<Real>("al", "al>0", "For this class, the capillary function is assumed to be saturation = exp(-(al*porepressure)^2) for porepressure<0.");
  params.addRequiredRangeCheckedParam<Real>("density0", "density0>0", "The density of the fluid phase");
  params.addRequiredRangeCheckedParam<Real>("bulk_modulus", "bulk_modulus>0", "The constant bulk modulus of the fluid phase");
  params.addClassDescription("This Material is used for the single-phase situation where log(mass-density) is the primary variable.  calculates the 1 porepressure and the 1 saturation in a 1-phase isothermal situation, and derivatives of these with respect to the PorousFlowVariables.  A gaussian capillary function is assumed");
  return params;
}

PorousFlowMaterial1PhaseMD_Gaussian::PorousFlowMaterial1PhaseMD_Gaussian(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _num_ph(1),
    _al(getParam<Real>("al")),
    _al2(std::pow(_al, 2)),
    _logdens0(std::log(getParam<Real>("density0"))),
    _bulk(getParam<Real>("bulk_modulus")),
    _recip_bulk(1.0/_al/_bulk),
    _recip_bulk2(std::pow(_recip_bulk, 2)),


    _md_var(coupledNodalValue("mass_density")),
    _qp_md_var(coupledValue("mass_density")),
    _gradmd_var(coupledGradient("mass_density")),
    _md_varnum(coupled("mass_density")),

    _dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),

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
    _dgrads_dv(declareProperty<std::vector<std::vector<RealGradient> > >("dPorousFlow_grad_saturation_dv"))
{
  if (_dictator_UO.num_phases() != 1)
    mooseError("The Dictator proclaims that the number of phases is " << _dictator_UO.num_phases() << " whereas PorousFlowMaterial1PhaseMD_Gaussian can only be used for 1-phase simulations.  Be aware that the Dictator has noted your mistake.");
}

void
PorousFlowMaterial1PhaseMD_Gaussian::initQpStatefulProperties()
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

  /*
   *  YAQI HACK !!
   *
   * I really just want to simply call
   * buildPS();
   * but i do it below in computeQpProperties
   */
  buildPS();
}

void
PorousFlowMaterial1PhaseMD_Gaussian::computeQpProperties()
{

  buildPS();

  /*
   *  YAQI HACK !!
   *
   * I really just want to simply call
   * buildPS();
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
    _dporepressure_dvar[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
    _dporepressure_qp_dvar[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
    _dgradp_dgradv[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
    _dgradp_dv[_qp][phase].assign(_dictator_UO.num_v(), RealGradient());
    _dsaturation_dvar[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
    _dsaturation_qp_dvar[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
    _dgrads_dgradv[_qp][phase].assign(_dictator_UO.num_v(), 0.0);
    _dgrads_dv[_qp][phase].assign(_dictator_UO.num_v(), RealGradient());
  }


  if (_dictator_UO.not_porflow_var(_md_varnum))
    return;

  const unsigned int pvar = _dictator_UO.porflow_var_num(_md_varnum);

  if (_md_var[_qp] >= _logdens0)
  {
    // fully saturated at the node
    _dporepressure_dvar[_qp][0][pvar] = _bulk;
    _dsaturation_dvar[_qp][0][pvar] = 0.0;
  }
  else
  {
    const Real pp = _porepressure[_qp][0];
    _dporepressure_dvar[_qp][0][pvar] = 1.0/(_recip_bulk - 2*_al*pp)/_al; //yes
    const Real sat = _saturation[_qp][0];
    _dsaturation_dvar[_qp][0][pvar] = -2*_al2*pp*sat*_dporepressure_dvar[_qp][0][pvar];
  }

  if (_qp_md_var[_qp] >= _logdens0)
  {
    // fully saturated at the quadpoint
    _dporepressure_qp_dvar[_qp][0][pvar] = _bulk;
    _dgradp_dgradv[_qp][0][pvar] = _bulk;
    _dgradp_dv[_qp][0][pvar] = 0.0;
    _dsaturation_qp_dvar[_qp][0][pvar] = 0.0;
    _dgrads_dgradv[_qp][0][pvar] = 0.0;
    _dgrads_dv[_qp][0][pvar] = 0.0;
  }
  else
  {
    const Real pp = _porepressure_qp[_qp][0];
    _dporepressure_qp_dvar[_qp][0][pvar] = 1.0/(_recip_bulk - 2*_al*pp)/_al; //yes
    _dgradp_dgradv[_qp][0][pvar] = 1.0/(_recip_bulk - 2*_al*pp)/_al; //yes
    _dgradp_dv[_qp][0][pvar] = _gradmd_var[_qp]*2*_al*_dporepressure_qp_dvar[_qp][0][pvar]/std::pow(_recip_bulk - 2*_al*_porepressure_qp[_qp][0], 2)/_al; //yes
    const Real sat = _saturation_qp[_qp][0];
    _dsaturation_qp_dvar[_qp][0][pvar] = -2*_al2*pp*sat*_dporepressure_qp_dvar[_qp][0][pvar];
    _dgrads_dgradv[_qp][0][pvar] = -2*_al2*_porepressure_qp[_qp][0]*_saturation_qp[_qp][0]*_dgradp_dgradv[_qp][0][pvar];
    _dgrads_dv[_qp][0][pvar] = -2*_al2*_dporepressure_qp_dvar[_qp][0][pvar]*_saturation_qp[_qp][0]*_gradp[_qp][0];
    _dgrads_dv[_qp][0][pvar] += -2*_al2*_porepressure_qp[_qp][0]*_dsaturation_qp_dvar[_qp][0][pvar]*_gradp[_qp][0];
    _dgrads_dv[_qp][0][pvar] += -2*_al2*_porepressure_qp[_qp][0]*_saturation_qp[_qp][0]*_dgradp_dv[_qp][0][pvar];
  }    
    
}

void
PorousFlowMaterial1PhaseMD_Gaussian::buildPS()
{
  if (_md_var[_qp] >= _logdens0)
  {
    // full saturation
    _porepressure[_qp][0] = (_md_var[_qp] - _logdens0)*_bulk;
    _saturation[_qp][0] = 1;
  }
  else
  {
    // v = logdens0 + p/bulk - (al p)^2
    // 0 = (v-logdens0) - p/bulk + (al p)^2
    // 2 al p = (1/al/bulk) +/- sqrt((1/al/bulk)^2 - 4(v-logdens0))  (the "minus" sign is chosen)
    // s = exp(-(al*p)^2)
    _porepressure[_qp][0] = (_recip_bulk - std::sqrt(_recip_bulk2 + 4*(_logdens0 - _md_var[_qp])))/(2*_al); //yes
    _saturation[_qp][0] = std::exp(-std::pow(_al*_porepressure[_qp][0], 2)); 
}

  if (_qp_md_var[_qp] >= _logdens0)
  {
    _porepressure_qp[_qp][0] = (_qp_md_var[_qp] - _logdens0)*_bulk;
    _gradp[_qp][0] = _gradmd_var[_qp]*_bulk;
    _saturation_qp[_qp][0] = 1;
    _grads[_qp][0] = 0;
  }
  else
  {
    _porepressure_qp[_qp][0] = (_recip_bulk - std::sqrt(_recip_bulk2 + 4*(_logdens0 - _qp_md_var[_qp])))/(2*_al); //yes
    _gradp[_qp][0] = _gradmd_var[_qp]/(_recip_bulk - 2*_al*_porepressure_qp[_qp][0])/_al; //yes
    _saturation_qp[_qp][0] = std::exp(-std::pow(_al*_porepressure_qp[_qp][0], 2));
    _grads[_qp][0] = -2*_al2*_porepressure_qp[_qp][0]*_saturation_qp[_qp][0]*_gradp[_qp][0];
  }  
}
