/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMass.h"

template <>
InputParameters
validParams<INSMass>()
{
  InputParameters params = validParams<INSBase>();

  params.addClassDescription("This class computes the mass equation residual and Jacobian "
                             "contributions for the incompressible Navier-Stokes momentum "
                             "equation.");
  params.addParam<bool>(
      "pspg", false, "Whether to perform PSPG stabilization of the mass equation");
  return params;
}

INSMass::INSMass(const InputParameters & parameters)
  : INSBase(parameters), _pspg(getParam<bool>("pspg"))
{
}

Real
INSMass::computeQpResidual()
{
  // (div u) * q
  // Note: we (arbitrarily) multiply this term by -1 so that it matches the -p(div v)
  // term in the momentum equation.  Not sure if that is really important?
  Real r = -(_grad_u_vel[_qp](0) + _grad_v_vel[_qp](1) + _grad_w_vel[_qp](2)) * _test[_i][_qp];

  if (_pspg)
    r += computeQpPGResidual();

  return r;
}

Real
INSMass::computeQpPGResidual()
{
  RealVectorValue viscous_term =
      _laplace ? strongViscousTermLaplace() : strongViscousTermTraction();
  RealVectorValue transient_term =
      _transient_term ? timeDerivativeTerm() : RealVectorValue(0, 0, 0);
  RealVectorValue convective_term = _convective_term ? convectiveTerm() : RealVectorValue(0, 0, 0);
  Real r = -tau() * _grad_test[_i][_qp] * (strongPressureTerm() + bodyForcesTerm() + viscous_term +
                                           convective_term + transient_term);

  return r;
}

Real
INSMass::computeQpJacobian()
{
  // Derivative wrt to p is zero
  Real r = 0;

  // Unless we are doing GLS stabilization
  if (_pspg)
    r += computeQpPGJacobian();

  return r;
}

Real
INSMass::computeQpPGJacobian()
{
  return -tau() * _grad_test[_i][_qp] * dStrongPressureDPressure();
}

Real
INSMass::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_var_number)
  {
    Real jac = -_grad_phi[_j][_qp](0) * _test[_i][_qp];
    if (_pspg)
      jac += computeQpPGOffDiagJacobian(0);
    return jac;
  }

  else if (jvar == _v_vel_var_number)
  {
    Real jac = -_grad_phi[_j][_qp](1) * _test[_i][_qp];
    if (_pspg)
      jac += computeQpPGOffDiagJacobian(1);
    return jac;
  }

  else if (jvar == _w_vel_var_number)
  {
    Real jac = -_grad_phi[_j][_qp](2) * _test[_i][_qp];
    if (_pspg)
      jac += computeQpPGOffDiagJacobian(2);
    return jac;
  }

  else
    return 0.0;
}

Real
INSMass::computeQpPGOffDiagJacobian(unsigned comp)
{
  RealVectorValue convective_term = _convective_term ? convectiveTerm() : RealVectorValue(0, 0, 0);
  RealVectorValue d_convective_term_d_u_comp =
      _convective_term ? dConvecDUComp(comp) : RealVectorValue(0, 0, 0);
  RealVectorValue viscous_term =
      _laplace ? strongViscousTermLaplace() : strongViscousTermTraction();
  RealVectorValue d_viscous_term_d_u_comp =
      _laplace ? dStrongViscDUCompLaplace(comp) : dStrongViscDUCompTraction(comp);
  RealVectorValue transient_term =
      _transient_term ? timeDerivativeTerm() : RealVectorValue(0, 0, 0);
  RealVectorValue d_transient_term_d_u_comp =
      _transient_term ? dTimeDerivativeDUComp(comp) : RealVectorValue(0, 0, 0);

  return -tau() * _grad_test[_i][_qp] *
             (d_convective_term_d_u_comp + d_viscous_term_d_u_comp + d_transient_term_d_u_comp) -
         dTauDUComp(comp) * _grad_test[_i][_qp] * (convective_term + viscous_term + transient_term +
                                                   strongPressureTerm() + bodyForcesTerm());
}
