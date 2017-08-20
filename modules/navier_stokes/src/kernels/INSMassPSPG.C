/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMassPSPG.h"

template <>
InputParameters
validParams<INSMassPSPG>()
{
  InputParameters params = validParams<INSBase>();

  params.addClassDescription(
      "This class computes the PSPG stabilization components for the incompressibility equation.");
  params.addParam<Real>("alpha", 1., "The alpha stabilization parameter.");
  params.addParam<bool>(
      "consistent",
      false,
      "Whether to consistently include the viscous term in the PSPG formulation.");
  return params;
}

INSMassPSPG::INSMassPSPG(const InputParameters & parameters)
  : INSBase(parameters), _alpha(getParam<Real>("alpha")), _consistent(getParam<bool>("consistent"))

{
}

Real
INSMassPSPG::computeQpResidual()
{
  Real tau = _alpha * _current_elem->hmax() * _current_elem->hmax() / (2. * _mu[_qp]);
  Real r = -tau * _grad_test[_i][_qp] * (computeStrongPressureTerm() + computeStrongGravityTerm());
  if (_consistent)
    r += -tau * _grad_test[_i][_qp] * computeStrongViscousTerm();
  if (!_stokes_only)
    r += -tau * _grad_test[_i][_qp] * computeStrongConvectiveTerm();

  return r;
}

Real
INSMassPSPG::computeQpJacobian()
{
  Real tau = _alpha * _current_elem->hmax() * _current_elem->hmax() / (2. * _mu[_qp]);
  return tau * _grad_test[_i][_qp] * dPressureDPressure();
}

Real
INSMassPSPG::computeQpOffDiagJacobian(unsigned jvar)
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
