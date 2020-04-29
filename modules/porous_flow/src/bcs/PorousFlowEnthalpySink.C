//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowEnthalpySink.h"
#include "Function.h"
#include "PorousFlowDictator.h"
#include "SinglePhaseFluidProperties.h"
#include "MooseVariable.h"

#include "libmesh/quadrature.h"

#include <iostream>

registerMooseObject("PorousFlowApp", PorousFlowEnthalpySink);

InputParameters
PorousFlowEnthalpySink::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addParam<unsigned int>(
      "fluid_phase",
      0,
      "If supplied, then this BC will potentially be a function of fluid pressure.");
  params.addRequiredParam<FunctionName>(
      "flux_function",
      "The mass flux.  The flux is OUT of the medium: hence positive values of "
      "this function means this BC will act as a SINK, while negative values "
      "indicate this flux will be a SOURCE.  The functional form is useful "
      "for spatially or temporally varying sinks.  This function is measured in kg.m^-2.s^-1.");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addRequiredCoupledVar("pressure", "Pressure");
  params.addRequiredParam<Real>("T_in", "Specified inlet temperature (measured in K)");
  params.addClassDescription(
      "Applies a source equal to the product of the mass flux and the "
      "fluid enthalpy. The enthalpy is computed at temperature T_in and pressure equal to the "
      "porepressure in the porous medium. Hence this adds heat energy "
      "to the porous medium at rate corresponding to a fluid being injected at (porepressure, "
      "T_in) at rate (-flux_function).");

  return params;
}

PorousFlowEnthalpySink::PorousFlowEnthalpySink(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _ph(getParam<unsigned int>("fluid_phase")),
    _m_func(getFunction("flux_function")),
    _pp(getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_nodal")),
    _dpp_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_porepressure_nodal_dvar")),
    _T_in(getParam<Real>("T_in")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
PorousFlowEnthalpySink::computeQpResidual()
{
  Real h = _fp.h_from_p_T(_pp[_i][_ph], _T_in);
  return _test[_i][_qp] * _m_func.value(_t, _q_point[_qp]) * h;
}

Real
PorousFlowEnthalpySink::computeQpJacobian()
{
  return jac(_var.number());
}

Real
PorousFlowEnthalpySink::computeQpOffDiagJacobian(unsigned int jvar)
{
  return jac(jvar);
}

Real
PorousFlowEnthalpySink::jac(unsigned int jvar) const
{
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;

  const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);

  if (_i != _j)
    return 0.0;
  Real h, dh_dpp, dh_dT;
  _fp.h_from_p_T(_pp[_i][_ph], _T_in, h, dh_dpp, dh_dT);
  Real jac = dh_dpp * _dpp_dvar[_i][_ph][pvar];
  return _test[_i][_qp] * _phi[_j][_qp] * jac;
}
