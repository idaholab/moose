//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes inclues
#include "NS.h"
#include "NSMomentumInviscidFluxWithGradP.h"

registerMooseObject("NavierStokesApp", NSMomentumInviscidFluxWithGradP);

InputParameters
NSMomentumInviscidFluxWithGradP::validParams()
{
  InputParameters params = NSKernel::validParams();
  params.addClassDescription(
      "This class computes the inviscid flux with pressure gradient in the momentum equation.");
  params.addRequiredCoupledVar(NS::pressure, "pressure");
  params.addRequiredParam<unsigned int>("component", "");
  return params;
}

NSMomentumInviscidFluxWithGradP::NSMomentumInviscidFluxWithGradP(const InputParameters & parameters)
  : NSKernel(parameters),
    _grad_p(coupledGradient(NS::pressure)),
    _component(getParam<unsigned int>("component")),
    _pressure_derivs(*this)
{
  // Store pointers to all variable gradients in a single vector.
  // This is needed for computing pressure Hessian values with a small
  // amount of code.
  _gradU.resize(5);
  _gradU[0] = &_grad_rho;
  _gradU[1] = &_grad_rho_u;
  _gradU[2] = &_grad_rho_v;
  _gradU[3] = &_grad_rho_w;
  _gradU[4] = &_grad_rho_et;
}

Real
NSMomentumInviscidFluxWithGradP::computeQpResidual()
{
  // For _component = k,

  // (rho*u) * u_k = (rho*u_k) * u <- we write it this way
  RealVectorValue vec(_u[_qp] * _u_vel[_qp],  // (U_k) * u_1
                      _u[_qp] * _v_vel[_qp],  // (U_k) * u_2
                      _u[_qp] * _w_vel[_qp]); // (U_k) * u_3

  // -((rho*u_k) * u) * grad(phi) + dp/dx_k * phi
  return -(vec * _grad_test[_i][_qp]) + _grad_p[_qp](_component) * _test[_i][_qp];
}

Real
NSMomentumInviscidFluxWithGradP::computeQpJacobian()
{
  RealVectorValue vec(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // The component'th entry of the on-diagonal Jacobian value is 2*u_i without the pressure
  // contribution.
  vec(_component) = 2. * vec(_component);

  // The Jacobian contribution due to grad(p) for the on-diagonal
  // variable, which is equal to _component+1.
  Real dFdp = pressureQpJacobianHelper(_component + 1);

  return
      // Convective terms Jacobian
      -(vec * _grad_test[_i][_qp]) * _phi[_j][_qp]
      // Pressure term Jacobian
      + dFdp * _test[_i][_qp];
}

Real
NSMomentumInviscidFluxWithGradP::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (isNSVariable(jvar))
  {

    // Map jvar into the numbering expected by this->compute_pressure_jacobain_value()
    unsigned int var_number = mapVarNumber(jvar);

    // The Jacobian contribution due to differentiating the grad(p)
    // term wrt variable var_number.
    Real dFdp = pressureQpJacobianHelper(var_number);

    if (jvar == _rho_var_number)
    {
      // Derivative of inviscid flux convective terms wrt density:
      // x-mom: (-u_1^2   , -u_1*u_2  , -u_1*u_3 ) * grad(phi_i) * phi_j
      // y-mom: (-u_2*u_1 , -u_2^2    , -u_2*u_3 ) * grad(phi_i) * phi_j
      // z-mom: (-u_3*u_1 , -u_3*u_2  , -u_3^2   ) * grad(phi_i) * phi_j

      // Start with the velocity vector
      RealVectorValue vec(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

      // Scale velocity vector by -1 * vec(_component)
      vec *= -vec(_component);

      return
          // Convective terms Jacobian
          -(vec * _grad_test[_i][_qp]) * _phi[_j][_qp]
          // Pressure term Jacobian
          + dFdp * _test[_i][_qp];
    }

    // Handle off-diagonal derivatives wrt momentums
    else if (jvar == _rhou_var_number || jvar == _rhov_var_number || jvar == _rhow_var_number)
    {
      // Start with the velocity vector
      RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

      // Map jvar into jlocal = {0,1,2}, regardless of how Moose has numbered things.
      // Can't do a case statement here since _rhou_var_number, etc. are not constants...
      unsigned int jlocal = 0;

      if (jvar == _rhov_var_number)
        jlocal = 1;
      else if (jvar == _rhow_var_number)
        jlocal = 2;

      return
          // Convective terms Jacobian
          -vel(_component) * _grad_test[_i][_qp](jlocal) * _phi[_j][_qp]
          // Pressure term Jacobian
          + dFdp * _test[_i][_qp];
    }

    else if (jvar == _rho_et_var_number)
    {
      // Pressure term Jacobian
      return dFdp * _test[_i][_qp];
    }
    else
      return 0.0;
  }
  else
    return 0.0;
}

Real
NSMomentumInviscidFluxWithGradP::pressureQpJacobianHelper(unsigned var_number)
{
  // Make sure our local gradient and Hessian data
  // structures are up-to-date for this quadrature point
  //  this->recalculate_gradient_and_hessian();

  Real hessian_sum = 0.0;
  for (unsigned int n = 0; n < 5; ++n)
    hessian_sum += _pressure_derivs.get_hess(var_number, n) * (*_gradU[n])[_qp](_component);

  // Hit hessian_sum with phij, then add to dp/dU_m * dphij/dx_k, finally return the result
  return _pressure_derivs.get_grad(var_number) * _grad_phi[_j][_qp](_component) +
         hessian_sum * _phi[_j][_qp];
}
