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
    _integrate_p_by_parts(getParam<bool>("integrate_p_by_parts")),
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

  return r;
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

  return jac;
}

Real
INSMomentumBase::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_var_number)
  {
    if (_convective_term)
      return _test[_i][_qp] * dConvecDUComp(0)(_component);
    else
      return 0;
    // return _test[_i][_qp] * (dConvecDUComp(0) + dViscDUComp(0))(_component);
  }
  else if (jvar == _v_vel_var_number)
  {
    if (_convective_term)
      return _test[_i][_qp] * dConvecDUComp(1)(_component);
    else
      return 0;
    // return _test[_i][_qp] * (dConvecDUComp(1) + dViscDUComp(1))(_component);
  }
  else if (jvar == _w_vel_var_number)
  {
    if (_convective_term)
      return _test[_i][_qp] * dConvecDUComp(2)(_component);
    else
      return 0;

    // return _test[_i][_qp] * (dConvecDUComp(2) + dViscDUComp(2))(_component);
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
