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
  InputParameters params = validParams<PorousFlowVariableBase>();
  params.addRequiredCoupledVar("mass_density", "Variable that represents log(mass-density) of the single phase");
  params.addRequiredRangeCheckedParam<Real>("al", "al>0", "For this class, the capillary function is assumed to be saturation = exp(-(al*porepressure)^2) for porepressure<0.");
  params.addRequiredRangeCheckedParam<Real>("density_P0", "density_P0>0", "The density of the fluid phase at zero porepressure");
  params.addRequiredRangeCheckedParam<Real>("bulk_modulus", "bulk_modulus>0", "The constant bulk modulus of the fluid phase");
  params.addClassDescription("This Material is used for the single-phase situation where log(mass-density) is the primary variable.  calculates the 1 porepressure and the 1 saturation in a 1-phase isothermal situation, and derivatives of these with respect to the PorousFlowVariables.  A gaussian capillary function is assumed");
  return params;
}

PorousFlowMaterial1PhaseMD_Gaussian::PorousFlowMaterial1PhaseMD_Gaussian(const InputParameters & parameters) :
    PorousFlowVariableBase(parameters),

    _al(getParam<Real>("al")),
    _al2(std::pow(_al, 2)),
    _logdens0(std::log(getParam<Real>("density_P0"))),
    _bulk(getParam<Real>("bulk_modulus")),
    _recip_bulk(1.0/_al/_bulk),
    _recip_bulk2(std::pow(_recip_bulk, 2)),

    _md_nodal_var(coupledNodalValue("mass_density")),
    _md_qp_var(coupledValue("mass_density")),
    _gradmd_qp_var(coupledGradient("mass_density")),
    _md_varnum(coupled("mass_density"))
{
  if (_dictator_UO.numPhases() != 1)
    mooseError("The Dictator proclaims that the number of phases is " << _dictator_UO.numPhases() << " whereas PorousFlowMaterial1PhaseMD_Gaussian can only be used for 1-phase simulations.  Be aware that the Dictator has noted your mistake.");
}

void
PorousFlowMaterial1PhaseMD_Gaussian::initQpStatefulProperties()
{
  PorousFlowVariableBase::initQpStatefulProperties();

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
  PorousFlowVariableBase::computeQpProperties();

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

  if (_dictator_UO.not_porflow_var(_md_varnum))
    return;

  const unsigned int pvar = _dictator_UO.porflow_var_num(_md_varnum);

  if (_md_nodal_var[_qp] >= _logdens0)
  {
    // fully saturated at the node
    _dporepressure_nodal_dvar[_qp][0][pvar] = _bulk;
    _dsaturation_nodal_dvar[_qp][0][pvar] = 0.0;
  }
  else
  {
    const Real pp = _porepressure_nodal[_qp][0];
    _dporepressure_nodal_dvar[_qp][0][pvar] = 1.0 / (_recip_bulk - 2.0 * _al * pp) / _al;
    const Real sat = _saturation_nodal[_qp][0];
    _dsaturation_nodal_dvar[_qp][0][pvar] = -2.0 * _al2 * pp * sat * _dporepressure_nodal_dvar[_qp][0][pvar];
  }

  if (_md_qp_var[_qp] >= _logdens0)
  {
    // fully saturated at the quadpoint
    _dporepressure_qp_dvar[_qp][0][pvar] = _bulk;
    _dgradp_qp_dgradv[_qp][0][pvar] = _bulk;
    _dgradp_qp_dv[_qp][0][pvar] = 0.0;
    _dsaturation_qp_dvar[_qp][0][pvar] = 0.0;
    _dgrads_qp_dgradv[_qp][0][pvar] = 0.0;
    _dgrads_qp_dv[_qp][0][pvar] = 0.0;
  }
  else
  {
    const Real pp = _porepressure_qp[_qp][0];
    _dporepressure_qp_dvar[_qp][0][pvar] = 1.0 / (_recip_bulk - 2.0 * _al * pp) / _al;
    _dgradp_qp_dgradv[_qp][0][pvar] = 1.0 / (_recip_bulk - 2.0 * _al * pp) / _al;
    _dgradp_qp_dv[_qp][0][pvar] = _gradmd_qp_var[_qp] * 2.0 * _al * _dporepressure_qp_dvar[_qp][0][pvar] / std::pow(_recip_bulk - 2.0 * _al * _porepressure_qp[_qp][0], 2.0) / _al;
    const Real sat = _saturation_qp[_qp][0];
    _dsaturation_qp_dvar[_qp][0][pvar] = -2.0 * _al2 * pp * sat * _dporepressure_qp_dvar[_qp][0][pvar];
    _dgrads_qp_dgradv[_qp][0][pvar] = -2.0 * _al2 * _porepressure_qp[_qp][0] * _saturation_qp[_qp][0] * _dgradp_qp_dgradv[_qp][0][pvar];
    _dgrads_qp_dv[_qp][0][pvar] = -2.0 * _al2 * _dporepressure_qp_dvar[_qp][0][pvar] * _saturation_qp[_qp][0] * _gradp_qp[_qp][0];
    _dgrads_qp_dv[_qp][0][pvar] += -2.0 * _al2 * _porepressure_qp[_qp][0] * _dsaturation_qp_dvar[_qp][0][pvar] * _gradp_qp[_qp][0];
    _dgrads_qp_dv[_qp][0][pvar] += -2.0 * _al2 * _porepressure_qp[_qp][0] * _saturation_qp[_qp][0] * _dgradp_qp_dv[_qp][0][pvar];
  }

  // _temperature is only dependent on _temperature, and its derivative is = 1
  if (!_dictator_UO.not_porflow_var(_temperature_varnum))
  {
    // _temperature is a PorousFlow variable
    _dtemperature_nodal_dvar[_qp][0][_dictator_UO.porflow_var_num(_temperature_varnum)] = 1.0;
    _dtemperature_qp_dvar[_qp][0][_dictator_UO.porflow_var_num(_temperature_varnum)] = 1.0;
  }
}

void
PorousFlowMaterial1PhaseMD_Gaussian::buildPS()
{
  if (_md_nodal_var[_qp] >= _logdens0)
  {
    // full saturation
    _porepressure_nodal[_qp][0] = (_md_nodal_var[_qp] - _logdens0) * _bulk;
    _saturation_nodal[_qp][0] = 1.0;
  }
  else
  {
    // v = logdens0 + p/bulk - (al p)^2
    // 0 = (v-logdens0) - p/bulk + (al p)^2
    // 2 al p = (1/al/bulk) +/- sqrt((1/al/bulk)^2 - 4(v-logdens0))  (the "minus" sign is chosen)
    // s = exp(-(al*p)^2)
    _porepressure_nodal[_qp][0] = (_recip_bulk - std::sqrt(_recip_bulk2 + 4.0 * (_logdens0 - _md_nodal_var[_qp]))) / (2.0 * _al);
    _saturation_nodal[_qp][0] = std::exp(-std::pow(_al * _porepressure_nodal[_qp][0], 2.0));
}

  if (_md_qp_var[_qp] >= _logdens0)
  {
    _porepressure_qp[_qp][0] = (_md_qp_var[_qp] - _logdens0) * _bulk;
    _gradp_qp[_qp][0] = _gradmd_qp_var[_qp] * _bulk;
    _saturation_qp[_qp][0] = 1.0;
    _grads_qp[_qp][0] = 0.0;
  }
  else
  {
    _porepressure_qp[_qp][0] = (_recip_bulk - std::sqrt(_recip_bulk2 + 4.0 * (_logdens0 - _md_qp_var[_qp]))) / (2.0 * _al);
    _gradp_qp[_qp][0] = _gradmd_qp_var[_qp] / (_recip_bulk - 2.0 * _al * _porepressure_qp[_qp][0]) / _al;
    _saturation_qp[_qp][0] = std::exp(-std::pow(_al * _porepressure_qp[_qp][0], 2.0));
    _grads_qp[_qp][0] = -2.0 * _al2 * _porepressure_qp[_qp][0] * _saturation_qp[_qp][0] * _gradp_qp[_qp][0];
  }

  /// Temperature is the same in each phase presently
  _temperature_nodal[_qp][0] = _temperature_nodal_var[_qp];
  _temperature_qp[_qp][0] = _temperature_qp_var[_qp];
}
