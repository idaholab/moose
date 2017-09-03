/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMomentumBase.h"

template <>
InputParameters
validParams<INSMomentumBase>()
{
  InputParameters params = validParams<INSBase>();

  params.addRequiredParam<unsigned>("component", "The velocity component that this is applied to.");
  params.addParam<bool>(
      "integrate_p_by_parts", true, "Whether to integrate the pressure term by parts.");
  return params;
}

INSMomentumBase::INSMomentumBase(const InputParameters & parameters)
  : INSBase(parameters),
    _component(getParam<unsigned>("component")),
    _integrate_p_by_parts(getParam<bool>("integrate_p_by_parts"))
{
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
  r += _test[_i][_qp] * bodyForcesTerm()(_component);

  // convective term
  if (_convective_term)
    r += _test[_i][_qp] * convectiveTerm()(_component);

  if (_stabilize)
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
  return tau() * U * _grad_test[_i][_qp] *
         (convective_term + viscous_term + strongPressureTerm() + bodyForcesTerm())(_component);

  // For GLS as opposed to SUPG stabilization, one would need to modify the test function functional
  // space to include second derivatives of the Galerkin test functions corresponding to the viscous
  // term. This would look like:
  // Real lap_test =
  //     _second_test[_i][_qp](0, 0) + _second_test[_i][_qp](1, 1) + _second_test[_i][_qp](2, 2);

  // Real pg_viscous_r = -_mu[_qp] * lap_test * tau() *
  //                     (convective_term + viscous_term + strongPressureTerm()(_component) +
  //                      bodyForcesTerm())(_component);
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

  if (_stabilize)
    jac += computeQpPGJacobian();

  return jac;
}

// Fix me
Real
INSMomentumBase::computeQpPGJacobian()
{
  return 0;
}

// Fix me
Real
INSMomentumBase::computeQpOffDiagJacobian(unsigned jvar)
{
  Real jac = 0;
  if (jvar == _u_vel_var_number)
  {
    return 0;
  }
  else if (jvar == _v_vel_var_number)
  {
    return 0;
  }
  else if (jvar == _w_vel_var_number)
  {
    return 0;
  }

  else if (jvar == _p_var_number)
  {
    if (_integrate_p_by_parts)
      jac += _grad_test[_i][_qp](_component) * dWeakPressureDPressure();
    else
      jac += _test[_i][_qp] * dStrongPressureDPressure()(_component);

    return jac;
  }

  else
    return 0;
}
