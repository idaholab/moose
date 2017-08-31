/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMomentumPSPG.h"
#include "MooseVariable.h"

template <>
InputParameters
validParams<INSMomentumPSPG>()
{
  InputParameters params = validParams<INSBase>();

  params.addClassDescription(
      "This class computes the PSPG stabilization components for the incompressibility equation.");
  params.addParam<Real>("alpha", 1., "The alpha stabilization parameter.");
  params.addParam<bool>(
      "consistent",
      true,
      "Whether to consistently include the viscous term in the PSPG formulation.");
  params.addParam<unsigned>("component", 0, "The velocity component");
  return params;
}

INSMomentumPSPG::INSMomentumPSPG(const InputParameters & parameters)
  : INSBase(parameters),
    _alpha(getParam<Real>("alpha")),
    _consistent(getParam<bool>("consistent")),
    _second_test(_var.secondPhi()),
    _component(getParam<unsigned>("component"))
{
}

Real
INSMomentumPSPG::computeQpResidual()
{
  Real tau = _alpha * _current_elem->hmax() * _current_elem->hmax() / (2. * _mu[_qp]);
  Real lap_test =
      _second_test[_i][_qp](0, 0) + _second_test[_i][_qp](1, 1) + _second_test[_i][_qp](2, 2);

  Real r = -_mu[_qp] * lap_test * tau *
           (computeStrongPressureTerm()(_component) + computeStrongGravityTerm()(_component));
  if (_consistent)
    r += -_mu[_qp] * lap_test * tau * computeStrongViscousTerm()(_component);
  // if (!_stokes_only)
  //   r += -_mu[_qp] * lap_test-tau * _grad_test[_i][_qp] * computeStrongConvectiveTerm();

  return r;
}

Real
INSMomentumPSPG::computeQpJacobian()
{
  Real tau = _alpha * _current_elem->hmax() * _current_elem->hmax() / (2. * _mu[_qp]);
  return tau * _grad_test[_i][_qp] * dPressureDPressure();
}

Real
INSMomentumPSPG::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_var_number)
  {
    Real tau = _alpha * _current_elem->hmax() * _current_elem->hmax() / (2. * _mu[_qp]);
    return tau * _grad_test[_i][_qp] * (/*dConvecDUComp(0) +*/ dViscDUComp(0));
  }
  else if (jvar == _v_vel_var_number)
  {
    Real tau = _alpha * _current_elem->hmax() * _current_elem->hmax() / (2. * _mu[_qp]);
    return tau * _grad_test[_i][_qp] * (/*dConvecDUComp(1) +*/ dViscDUComp(1));
  }
  else if (jvar == _w_vel_var_number)
  {
    Real tau = _alpha * _current_elem->hmax() * _current_elem->hmax() / (2. * _mu[_qp]);
    return tau * _grad_test[_i][_qp] * (/*dConvecDUComp(2) +*/ dViscDUComp(2));
  }
  else
    return 0;
}
