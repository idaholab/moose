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
  return params;
}

INSMassPSPG::INSMassPSPG(const InputParameters & parameters)
  : INSBase(parameters), _alpha(getParam<Real>("alpha"))

{
}

Real
INSMassPSPG::computeQpResidual()
{
  Real tau = _alpha * _current_elem->hmax() * _current_elem->hmax() / (2. * _mu[_qp]);
  return -tau * _grad_test[_i][_qp] *
         (/*computeStrongConvectiveTerm() + computeStrongViscousTerm() +*/
          computeStrongPressureTerm() + computeStrongGravityTerm());
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
