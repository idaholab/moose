//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDesorpedMassTimeDerivative.h"

#include "MooseVariable.h"

#include "libmesh/quadrature.h"

registerMooseObject("PorousFlowApp", PorousFlowDesorpedMassTimeDerivative);

InputParameters
PorousFlowDesorpedMassTimeDerivative::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addRequiredCoupledVar(
      "conc_var", "The variable that represents the concentration of desorped species");
  params.addClassDescription("Desorped component mass derivative wrt time.");
  return params;
}

PorousFlowDesorpedMassTimeDerivative::PorousFlowDesorpedMassTimeDerivative(
    const InputParameters & parameters)
  : TimeKernel(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _conc_var_number(coupled("conc_var")),
    _conc(coupledValue("conc_var")),
    _conc_old(coupledValueOld("conc_var")),
    _porosity(getMaterialProperty<Real>("PorousFlow_porosity_qp")),
    _porosity_old(getMaterialPropertyOld<Real>("PorousFlow_porosity_qp")),
    _dporosity_dvar(getMaterialProperty<std::vector<Real>>("dPorousFlow_porosity_qp_dvar")),
    _dporosity_dgradvar(
        getMaterialProperty<std::vector<RealGradient>>("dPorousFlow_porosity_qp_dgradvar"))
{
}

Real
PorousFlowDesorpedMassTimeDerivative::computeQpResidual()
{
  Real c = (1.0 - _porosity[_qp]) * _conc[_qp];
  Real c_old = (1.0 - _porosity_old[_qp]) * _conc_old[_qp];
  return _test[_i][_qp] * (c - c_old) / _dt;
}

Real
PorousFlowDesorpedMassTimeDerivative::computeQpJacobian()
{
  return computeQpJac(_var.number());
}

Real
PorousFlowDesorpedMassTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  return computeQpJac(jvar);
}

Real
PorousFlowDesorpedMassTimeDerivative::computeQpJac(unsigned int jvar) const
{
  Real deriv = 0.0;

  if (jvar == _conc_var_number)
    deriv = (1.0 - _porosity[_qp]) * _phi[_j][_qp];

  if (_dictator.notPorousFlowVariable(jvar))
    return _test[_i][_qp] * deriv / _dt;
  const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);

  deriv -= _dporosity_dgradvar[_qp][pvar] * _grad_phi[_j][_qp] * _conc[_qp];
  deriv -= _dporosity_dvar[_qp][pvar] * _phi[_j][_qp] * _conc[_qp];

  return _test[_i][_qp] * deriv / _dt;
}
