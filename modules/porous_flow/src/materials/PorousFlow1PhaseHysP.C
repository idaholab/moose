//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlow1PhaseHysP.h"

registerMooseObject("PorousFlowApp", PorousFlow1PhaseHysP);

InputParameters
PorousFlow1PhaseHysP::validParams()
{
  InputParameters params = PorousFlowHystereticCapillaryPressure::validParams();
  params.addRequiredCoupledVar(
      "porepressure", "Variable that represents the porepressure of the single liquid phase");
  params.addClassDescription(
      "This Material is used for unsaturated single-phase situations "
      "where porepressure is the primary variable and the capillary pressure is hysteretic.  The "
      "hysteretic formulation assumes that the single phase is a liquid");
  return params;
}

PorousFlow1PhaseHysP::PorousFlow1PhaseHysP(const InputParameters & parameters)
  : PorousFlowHystereticCapillaryPressure(parameters),
    _pc(_nodal_material ? declareProperty<Real>("PorousFlow_hysteretic_capillary_pressure_nodal")
                        : declareProperty<Real>("PorousFlow_hysteretic_capillary_pressure_qp")),
    _porepressure_var(_nodal_material ? coupledDofValues("porepressure")
                                      : coupledValue("porepressure")),
    _gradp_qp_var(coupledGradient("porepressure")),
    _porepressure_varnum(coupled("porepressure")),
    _p_var_num(_dictator.isPorousFlowVariable(_porepressure_varnum)
                   ? _dictator.porousFlowVariableNum(_porepressure_varnum)
                   : 0)
{
  if (_num_phases != 1)
    mooseError("The Dictator proclaims that the number of phases is ",
               _dictator.numPhases(),
               " whereas PorousFlow1PhaseHysP can only be used for 1-phase simulations.  Be aware "
               "that the Dictator has noted your mistake.");
}

void
PorousFlow1PhaseHysP::initQpStatefulProperties()
{
  PorousFlowHystereticCapillaryPressure::initQpStatefulProperties();
  buildQpPPSS();
}

void
PorousFlow1PhaseHysP::computeQpProperties()
{
  // size stuff correctly and prepare the derivative matrices with zeroes
  PorousFlowHystereticCapillaryPressure::computeQpProperties();

  buildQpPPSS();
  const Real pc = -_porepressure_var[_qp];
  const Real ds = -dliquidSaturationQp(pc);

  if (!_nodal_material)
  {
    (*_gradp_qp)[_qp][0] = _gradp_qp_var[_qp];
    (*_grads_qp)[_qp][0] = ds * _gradp_qp_var[_qp];
  }

  // _porepressure is only dependent on _porepressure, and its derivative is 1
  if (_dictator.isPorousFlowVariable(_porepressure_varnum))
  {
    // _porepressure is a PorousFlow variable
    (*_dporepressure_dvar)[_qp][0][_p_var_num] = 1.0;
    (*_dsaturation_dvar)[_qp][0][_p_var_num] = ds;
    if (!_nodal_material)
    {
      (*_dgradp_qp_dgradv)[_qp][0][_p_var_num] = 1.0;
      (*_dgrads_qp_dgradv)[_qp][0][_p_var_num] = ds;
      (*_dgrads_qp_dv)[_qp][0][_p_var_num] = d2liquidSaturationQp(pc) * _gradp_qp_var[_qp];
    }
  }
}

void
PorousFlow1PhaseHysP::buildQpPPSS()
{
  _porepressure[_qp][0] = _porepressure_var[_qp];
  _pc[_qp] = -_porepressure_var[_qp];
  _saturation[_qp][0] = liquidSaturationQp(_pc[_qp]);
}
