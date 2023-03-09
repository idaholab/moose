//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlow1PhaseMD_Gaussian.h"

registerMooseObject("PorousFlowApp", PorousFlow1PhaseMD_Gaussian);

InputParameters
PorousFlow1PhaseMD_Gaussian::validParams()
{
  InputParameters params = PorousFlowVariableBase::validParams();
  params.addRequiredCoupledVar("mass_density",
                               "Variable that represents log(mass-density) of the single phase");
  params.addRequiredRangeCheckedParam<Real>(
      "al",
      "al>0",
      "For this class, the capillary function is assumed to be saturation = "
      "exp(-(al*porepressure)^2) for porepressure<0.");
  params.addRequiredRangeCheckedParam<Real>(
      "density_P0", "density_P0>0", "The density of the fluid phase at zero porepressure");
  params.addRequiredRangeCheckedParam<Real>(
      "bulk_modulus", "bulk_modulus>0", "The constant bulk modulus of the fluid phase");
  params.addClassDescription("This Material is used for the single-phase situation where "
                             "log(mass-density) is the primary variable.  calculates the 1 "
                             "porepressure and the 1 saturation in a 1-phase situation, "
                             "and derivatives of these with respect to the PorousFlowVariables.  A "
                             "gaussian capillary function is assumed");
  return params;
}

PorousFlow1PhaseMD_Gaussian::PorousFlow1PhaseMD_Gaussian(const InputParameters & parameters)
  : PorousFlowVariableBase(parameters),

    _al(getParam<Real>("al")),
    _al2(std::pow(_al, 2)),
    _logdens0(std::log(getParam<Real>("density_P0"))),
    _bulk(getParam<Real>("bulk_modulus")),
    _recip_bulk(1.0 / _al / _bulk),
    _recip_bulk2(std::pow(_recip_bulk, 2)),

    _md_var(_nodal_material ? coupledDofValues("mass_density") : coupledValue("mass_density")),
    _gradmd_qp_var(coupledGradient("mass_density")),
    _md_varnum(coupled("mass_density")),
    _pvar(_dictator.isPorousFlowVariable(_md_varnum) ? _dictator.porousFlowVariableNum(_md_varnum)
                                                     : 0)
{
  if (_num_phases != 1)
    mooseError("The Dictator proclaims that the number of phases is ",
               _dictator.numPhases(),
               " whereas PorousFlow1PhaseMD_Gaussian can only be used for 1-phase simulations.  Be "
               "aware that the Dictator has noted your mistake.");
}

void
PorousFlow1PhaseMD_Gaussian::initQpStatefulProperties()
{
  PorousFlowVariableBase::initQpStatefulProperties();
  buildPS();
}

void
PorousFlow1PhaseMD_Gaussian::computeQpProperties()
{
  // size stuff correctly and prepare the derivative matrices with zeroes
  PorousFlowVariableBase::computeQpProperties();

  buildPS();

  if (!_nodal_material)
  {
    if (_md_var[_qp] >= _logdens0)
    {
      (*_gradp_qp)[_qp][0] = _gradmd_qp_var[_qp] * _bulk;
      (*_grads_qp)[_qp][0] = 0.0;
    }
    else
    {
      (*_gradp_qp)[_qp][0] =
          _gradmd_qp_var[_qp] / (_recip_bulk - 2.0 * _al * _porepressure[_qp][0]) / _al;
      (*_grads_qp)[_qp][0] =
          -2.0 * _al2 * _porepressure[_qp][0] * _saturation[_qp][0] * (*_gradp_qp)[_qp][0];
    }
  }

  if (_dictator.notPorousFlowVariable(_md_varnum))
    return;

  if (_md_var[_qp] >= _logdens0)
  {
    // fully saturated at the node or quadpoint
    (*_dporepressure_dvar)[_qp][0][_pvar] = _bulk;
    (*_dsaturation_dvar)[_qp][0][_pvar] = 0.0;
  }
  else
  {
    const Real pp = _porepressure[_qp][0];
    (*_dporepressure_dvar)[_qp][0][_pvar] = 1.0 / (_recip_bulk - 2.0 * _al * pp) / _al;
    const Real sat = _saturation[_qp][0];
    (*_dsaturation_dvar)[_qp][0][_pvar] =
        -2.0 * _al2 * pp * sat * (*_dporepressure_dvar)[_qp][0][_pvar];
  }

  if (!_nodal_material)
  {
    if (_md_var[_qp] >= _logdens0)
    {
      // fully saturated at the quadpoint
      (*_dgradp_qp_dgradv)[_qp][0][_pvar] = _bulk;
      (*_dgradp_qp_dv)[_qp][0][_pvar] = 0.0;
      (*_dgrads_qp_dgradv)[_qp][0][_pvar] = 0.0;
      (*_dgrads_qp_dv)[_qp][0][_pvar] = 0.0;
    }
    else
    {
      const Real pp = _porepressure[_qp][0];
      (*_dgradp_qp_dgradv)[_qp][0][_pvar] = 1.0 / (_recip_bulk - 2.0 * _al * pp) / _al;
      (*_dgradp_qp_dv)[_qp][0][_pvar] = _gradmd_qp_var[_qp] * 2.0 * _al *
                                        (*_dporepressure_dvar)[_qp][0][_pvar] /
                                        std::pow(_recip_bulk - 2.0 * _al * pp, 2.0) / _al;
      const Real sat = _saturation[_qp][0];
      (*_dgrads_qp_dgradv)[_qp][0][_pvar] =
          -2.0 * _al2 * pp * sat * (*_dgradp_qp_dgradv)[_qp][0][_pvar];
      (*_dgrads_qp_dv)[_qp][0][_pvar] =
          -2.0 * _al2 * (*_dporepressure_dvar)[_qp][0][_pvar] * sat * (*_gradp_qp)[_qp][0];
      (*_dgrads_qp_dv)[_qp][0][_pvar] +=
          -2.0 * _al2 * pp * (*_dsaturation_dvar)[_qp][0][_pvar] * (*_gradp_qp)[_qp][0];
      (*_dgrads_qp_dv)[_qp][0][_pvar] += -2.0 * _al2 * pp * sat * (*_dgradp_qp_dv)[_qp][0][_pvar];
    }
  }
}

void
PorousFlow1PhaseMD_Gaussian::buildPS()
{
  if (_md_var[_qp] >= _logdens0)
  {
    // full saturation
    _porepressure[_qp][0] = (_md_var[_qp] - _logdens0) * _bulk;
    _saturation[_qp][0] = 1.0;
  }
  else
  {
    // v = logdens0 + p/bulk - (al p)^2
    // 0 = (v-logdens0) - p/bulk + (al p)^2
    // 2 al p = (1/al/bulk) +/- sqrt((1/al/bulk)^2 - 4(v-logdens0))  (the "minus" sign is chosen)
    // s = exp(-(al*p)^2)
    _porepressure[_qp][0] =
        (_recip_bulk - std::sqrt(_recip_bulk2 + 4.0 * (_logdens0 - _md_var[_qp]))) / (2.0 * _al);
    _saturation[_qp][0] = std::exp(-std::pow(_al * _porepressure[_qp][0], 2.0));
  }
}
