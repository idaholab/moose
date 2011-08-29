#include "NSMomentumInviscidFluxWithGradP.h"
 

template<>
InputParameters validParams<NSMomentumInviscidFluxWithGradP>()
{
  InputParameters params = validParams<NSKernel>();

  // Coupled variables
  params.addRequiredCoupledVar("pressure", "");

  // Required parameters
  params.addRequiredParam<Real>("component", "");

  return params;
}





NSMomentumInviscidFluxWithGradP::NSMomentumInviscidFluxWithGradP(const std::string & name, InputParameters parameters)
  : NSKernel(name, parameters),
    
   // Coupled gradients
   _grad_p(coupledGradient("pressure")),
   
   // Parameters
   _component(getParam<Real>("component"))
{
  // Zero out the gradient and Hessian entries so we are never dealing with uninitialized memory
  for (unsigned i=0; i<5; ++i)
  {
    _dpdU[i] = 0.;
    for (unsigned j=0; j<5; ++j)
      _hessian[i][j] = 0.;
  }

  // Store pointers to all variable gradients in a single vector.
  // This is needed for computing pressure Hessian values with a small 
  // amount of code.
  _gradU.resize(5);
  _gradU[0] = &_grad_rho  ;
  _gradU[1] = &_grad_rho_u;
  _gradU[2] = &_grad_rho_v;
  _gradU[3] = &_grad_rho_w;
  _gradU[4] = &_grad_rho_e;
}




Real
NSMomentumInviscidFluxWithGradP::computeQpResidual()
{
  // For _component = k,
  
  // (rho*u) * u_k = (rho*u_k) * u <- we write it this way
  RealVectorValue vec(_u[_qp]*_u_vel[_qp],   // (U_k) * u_1
		      _u[_qp]*_v_vel[_qp],   // (U_k) * u_2
		      _u[_qp]*_w_vel[_qp]);  // (U_k) * u_3

  // -((rho*u_k) * u) * grad(phi) + dp/dx_k * phi
  return -(vec*_grad_test[_i][_qp]) + _grad_p[_qp](_component)*_test[_i][_qp];
}




Real
NSMomentumInviscidFluxWithGradP::computeQpJacobian()
{
  RealVectorValue vec(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  
  // The component'th entry of the on-diagonal Jacobian value is 2*u_i without the pressure
  // contribution.  
  vec(_component) = 2.*vec(_component);

  // The Jacobian contribution due to grad(p) for the on-diagonal 
  // variable, which is equal to _component+1.
  Real dFdp = compute_pressure_jacobian_value(_component+1);

  return 
    // Convective terms Jacobian
    -(vec * _grad_test[_i][_qp]) * _phi[_j][_qp] 

    +
    
    // Pressure term Jacobian
    dFdp*_test[_i][_qp];
}





Real
NSMomentumInviscidFluxWithGradP::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Map jvar into the numbering expected by this->compute_pressure_jacobain_value()
  unsigned var_number = this->map_var_number(jvar);

  // The Jacobian contribution due to differentiating the grad(p)
  // term wrt variable var_number.
  Real dFdp = compute_pressure_jacobian_value(var_number);
  

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
      
      +
      
      // Pressure term Jacobian
      dFdp*_test[_i][_qp];
  }

  
  // Handle off-diagonal derivatives wrt momentums
  else if ((jvar == _rhou_var_number) || (jvar == _rhov_var_number) || (jvar == _rhow_var_number))
  {
    // Start with the velocity vector
    RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

    // Map jvar into jlocal = {0,1,2}, regardless of how Moose has numbered things.
    // Can't do a case statement here since _rhou_var_number, etc. are not constants...
    unsigned jlocal = 0;
    
    if (jvar == _rhov_var_number)
      jlocal = 1;
    else if (jvar == _rhow_var_number)
      jlocal = 2;

    return
      // Convective terms Jacobian
      -vel(_component) * _grad_test[_i][_qp](jlocal) * _phi[_j][_qp]

      + 
      
      // Pressure term Jacobian
      dFdp*_test[_i][_qp];
  }

  else if (jvar == _rhoe_var_number)
  {
    return 
      // Pressure term Jacobian
      dFdp*_test[_i][_qp];
  }

  // We shouldn't get here... jvar should have matched one of the if statements above!
  mooseError("computeQpOffDiagJacobian called with invalid jvar.");

  return 0;
}





Real NSMomentumInviscidFluxWithGradP::compute_pressure_jacobian_value(unsigned var_number)
{
  // Make sure our local gradient and Hessian data 
  // structures are up-to-date for this quadrature point
  this->recalculate_gradient_and_hessian();

  Real hessian_sum = 0.;
  for (unsigned n=0; n<5; ++n)
    hessian_sum += this->get_hess(var_number,n) * (*_gradU[n])[_qp](_component);

  // Hit hessian_sum with phij, then add to dp/dU_m * dphij/dx_k, finally return the result
  return _dpdU[var_number]*_grad_phi[_j][_qp](_component) + hessian_sum*_phi[_j][_qp];
}




void NSMomentumInviscidFluxWithGradP::recalculate_gradient_and_hessian()
{
  // Convenience variables
  Real U0 = _rho[_qp];

  Real u = _u_vel[_qp];
  Real v = _v_vel[_qp];
  Real w = _w_vel[_qp];
  
  Real vel2 = (u*u + v*v + w*w);


  // Recompute gradient entries at the current qp.
  _dpdU[0] = 0.5*(_gamma-1.) * vel2;
  _dpdU[1] = (1.-_gamma) * u;
  _dpdU[2] = (1.-_gamma) * v;
  _dpdU[3] = (1.-_gamma) * w;
  _dpdU[4] = _gamma - 1.;

  // Recompute Hessian entries at the current qp.  Only the lower triangle is filled in,
  // the matrix is symmetric.

  const Real tmp = (1.-_gamma)/U0;
  
  // Row 0
  _hessian[0][0] = tmp*vel2;

  // Row 1
  _hessian[1][0] = -tmp*u;
  _hessian[1][1] = tmp;

  // Row 2
  _hessian[2][0] = -tmp*v;
  _hessian[2][2] = tmp;

  // Row 3
  _hessian[3][0] = -tmp*w;
  _hessian[3][3] = tmp;
  
  // Row 4 is all zeros
}


Real NSMomentumInviscidFluxWithGradP::get_hess(unsigned i, unsigned j)
{
  // If in lower triangle, OK, return value directly
  if (i>=j)
    return _hessian[i][j];

  // Otherwise, convert passed in upper-triangular entry to
  // lower-triangular.
  return _hessian[j][i];
}
