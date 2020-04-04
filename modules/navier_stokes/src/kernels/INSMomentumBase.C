//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSMomentumBase.h"
#include "Function.h"

InputParameters
INSMomentumBase::validParams()
{
  InputParameters params = INSBase::validParams();

  params.addRequiredParam<unsigned>("component", "The velocity component that this is applied to.");
  params.addParam<bool>(
      "integrate_p_by_parts", true, "Whether to integrate the pressure term by parts.");
  params.addParam<bool>(
      "supg", false, "Whether to perform SUPG stabilization of the momentum residuals");
  params.addParam<FunctionName>("forcing_func", 0, "The mms forcing function.");
  return params;
}

INSMomentumBase::INSMomentumBase(const InputParameters & parameters)
  : INSBase(parameters),
    _component(getParam<unsigned>("component")),
    _integrate_p_by_parts(getParam<bool>("integrate_p_by_parts")),
    _supg(getParam<bool>("supg")),
    _ffn(getFunction("forcing_func"))
{
  if (_supg && !_convective_term)
    mooseError("It doesn't make sense to conduct SUPG stabilization without a convective term.");
}

Real
INSMomentumBase::computeQpResidual()
{
  Real r = 0;

  // viscous term
  r += computeQpResidualViscousPart();

  // pressure term
  if (_integrate_p_by_parts)
    r += _grad_test[_i][_qp](_component) * weakPressureTerm();
  else
    r += _test[_i][_qp] * strongPressureTerm()(_component);

  // body force term
  r += _test[_i][_qp] * (gravityTerm()(_component) - _ffn.value(_t, _q_point[_qp]));

  // convective term
  if (_convective_term)
    r += _test[_i][_qp] * convectiveTerm()(_component);

  if (_supg)
    r += computeQpPGResidual();

  return r;
}

Real
INSMomentumBase::computeQpPGResidual()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  RealVectorValue convective_term = _convective_term ? convectiveTerm() : RealVectorValue(0, 0, 0);
  RealVectorValue viscous_term =
      _laplace ? strongViscousTermLaplace() : strongViscousTermTraction();
  RealVectorValue transient_term =
      _transient_term ? timeDerivativeTerm() : RealVectorValue(0, 0, 0);

  return tau() * U * _grad_test[_i][_qp] *
         ((convective_term + viscous_term + transient_term + strongPressureTerm() +
           gravityTerm())(_component)-_ffn.value(_t, _q_point[_qp]));

  // For GLS as opposed to SUPG stabilization, one would need to modify the test function functional
  // space to include second derivatives of the Galerkin test functions corresponding to the viscous
  // term. This would look like:
  // Real lap_test =
  //     _second_test[_i][_qp](0, 0) + _second_test[_i][_qp](1, 1) + _second_test[_i][_qp](2, 2);

  // Real pg_viscous_r = -_mu[_qp] * lap_test * tau() *
  //                     (convective_term + viscous_term + strongPressureTerm()(_component) +
  //                      gravityTerm())(_component);
}

Real
INSMomentumBase::computeQpJacobian()
{
  Real jac = 0;

  // viscous term
  jac += computeQpJacobianViscousPart();

  // convective term
  if (_convective_term)
    jac += _test[_i][_qp] * dConvecDUComp(_component)(_component);

  if (_supg)
    jac += computeQpPGJacobian(_component);

  return jac;
}

Real
INSMomentumBase::computeQpPGJacobian(unsigned comp)
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  RealVectorValue d_U_d_U_comp(0, 0, 0);
  d_U_d_U_comp(comp) = _phi[_j][_qp];

  Real convective_term = _convective_term ? convectiveTerm()(_component) : 0;
  Real d_convective_term_d_u_comp = _convective_term ? dConvecDUComp(comp)(_component) : 0;
  Real viscous_term =
      _laplace ? strongViscousTermLaplace()(_component) : strongViscousTermTraction()(_component);
  Real d_viscous_term_d_u_comp = _laplace ? dStrongViscDUCompLaplace(comp)(_component)
                                          : dStrongViscDUCompTraction(comp)(_component);
  Real transient_term = _transient_term ? timeDerivativeTerm()(_component) : 0;
  Real d_transient_term_d_u_comp = _transient_term ? dTimeDerivativeDUComp(comp)(_component) : 0;

  return dTauDUComp(comp) * U * _grad_test[_i][_qp] *
             (convective_term + viscous_term + strongPressureTerm()(_component) +
              gravityTerm()(_component) + transient_term - _ffn.value(_t, _q_point[_qp])) +
         tau() * d_U_d_U_comp * _grad_test[_i][_qp] *
             (convective_term + viscous_term + strongPressureTerm()(_component) +
              gravityTerm()(_component) + transient_term - _ffn.value(_t, _q_point[_qp])) +
         tau() * U * _grad_test[_i][_qp] *
             (d_convective_term_d_u_comp + d_viscous_term_d_u_comp + d_transient_term_d_u_comp);
}

Real
INSMomentumBase::computeQpOffDiagJacobian(unsigned jvar)
{
  Real jac = 0;
  if (jvar == _u_vel_var_number)
  {
    Real convective_term = _convective_term ? _test[_i][_qp] * dConvecDUComp(0)(_component) : 0.;
    Real viscous_term = computeQpOffDiagJacobianViscousPart(jvar);

    jac += convective_term + viscous_term;

    if (_supg)
      jac += computeQpPGJacobian(0);

    return jac;
  }
  else if (jvar == _v_vel_var_number)
  {
    Real convective_term = _convective_term ? _test[_i][_qp] * dConvecDUComp(1)(_component) : 0.;
    Real viscous_term = computeQpOffDiagJacobianViscousPart(jvar);

    jac += convective_term + viscous_term;

    if (_supg)
      jac += computeQpPGJacobian(1);

    return jac;
  }
  else if (jvar == _w_vel_var_number)
  {
    Real convective_term = _convective_term ? _test[_i][_qp] * dConvecDUComp(2)(_component) : 0.;
    Real viscous_term = computeQpOffDiagJacobianViscousPart(jvar);

    jac += convective_term + viscous_term;

    if (_supg)
      jac += computeQpPGJacobian(2);

    return jac;
  }

  else if (jvar == _p_var_number)
  {
    if (_integrate_p_by_parts)
      jac += _grad_test[_i][_qp](_component) * dWeakPressureDPressure();
    else
      jac += _test[_i][_qp] * dStrongPressureDPressure()(_component);

    if (_supg)
    {
      RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
      jac += tau() * U * _grad_test[_i][_qp] * dStrongPressureDPressure()(_component);
    }

    return jac;
  }

  else
    return 0;
}
