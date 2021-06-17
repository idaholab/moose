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
#include "PorousFlowSinkBC.h"

#include "libmesh/quadrature.h"

#include <iostream>

registerMooseObject("PorousFlowApp", PorousFlowEnthalpySink);

InputParameters
PorousFlowEnthalpySink::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params += PorousFlowSinkBC::validParamsCommon();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addClassDescription(
      "Applies a source equal to the product of the mass flux and the "
      "fluid enthalpy. The enthalpy is computed at temperature T_in and pressure equal to the "
      "porepressure in the porous medium, if fluid_phase is given, otherwise at the supplied "
      "porepressure. Hence this adds heat energy to the porous medium at rate corresponding to a "
      "fluid being injected at (porepressure, T_in) at rate (-flux_function).");

  return params;
}

PorousFlowEnthalpySink::PorousFlowEnthalpySink(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _pressure(isCoupled("porepressure_var") ? &coupledValue("porepressure_var") : nullptr),
    _ph(isParamValid("fluid_phase") ? getParam<unsigned int>("fluid_phase")
                                    : libMesh::invalid_uint),
    _m_func(getFunction("flux_function")),
    _pp(getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_nodal")),
    _dpp_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_porepressure_nodal_dvar")),
    _T_in(getParam<Real>("T_in")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
  if ((_pressure != nullptr) && (isParamValid("fluid_phase")))
    mooseError(name(), ": Cannot specify both pressure and pore pressure.");

  if ((_pressure == nullptr) && (!isParamValid("fluid_phase")))
    mooseError(name(), ": You have to specify either 'pressure' or 'fluid_phase'.");

  if (isParamValid("fluid_phase"))
    if (_ph >= _dictator.numPhases())
      mooseError(name(),
                 ": Specified 'fluid_phase' is larger than the number of phases available in the "
                 "simulation (",
                 _dictator.numPhases(),
                 ").");
}

Real
PorousFlowEnthalpySink::computeQpResidual()
{
  Real h;
  if (_pressure)
    h = _fp.h_from_p_T((*_pressure)[_qp], _T_in);
  else
    h = _fp.h_from_p_T(_pp[_i][_ph], _T_in);

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

  Real jac;
  if (_pressure)
  {
    jac = 0.;
  }
  else
  {
    Real h, dh_dpp, dh_dT;
    _fp.h_from_p_T(_pp[_i][_ph], _T_in, h, dh_dpp, dh_dT);
    jac = dh_dpp * _dpp_dvar[_i][_ph][pvar];
  }
  return _test[_i][_qp] * _m_func.value(_t, _q_point[_qp]) * jac;
}
