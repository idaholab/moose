#include "NSMomentumInviscidFlux.h"
 

template<>
InputParameters validParams<NSMomentumInviscidFlux>()
{
  InputParameters params = validParams<Kernel>();

  // Required parameters
  params.addRequiredParam<Real>("component", "");
  
  // Required copuled variables
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", "");
  params.addRequiredCoupledVar("pressure", "");

  // Required parameters
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats");

  // Required "coupled" variables, only the numerical index of these
  // variables is required, not their actual value.
  params.addRequiredCoupledVar("rho", "");
  params.addRequiredCoupledVar("rhou", "");
  params.addRequiredCoupledVar("rhov", "");
  params.addCoupledVar("rhow", ""); // only required in 3D
  params.addRequiredCoupledVar("rhoe", "");

  return params;
}





NSMomentumInviscidFlux::NSMomentumInviscidFlux(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   // Coupled variables
   _u_vel(coupledValue("u")),
   _v_vel(coupledValue("v")),
   _w_vel(_dim == 3 ? coupledValue("w") : _zero),
   _pressure(coupledValue("pressure")),
   
   // Parameters
   _component(getParam<Real>("component")),
   _gamma(getParam<Real>("gamma")),

   // Variable numbers
   _rho_var_number( coupled("rho") ),
   _rhou_var_number( coupled("rhou") ),
   _rhov_var_number( coupled("rhov") ),
   _rhow_var_number( _dim == 3 ? coupled("rhow") : libMesh::invalid_uint),
   _rhoe_var_number( coupled("rhoe") )
{}




Real
NSMomentumInviscidFlux::computeQpResidual()
{
  // For _component = k,
  
  // (rho*u) * u_k = (rho*u_k) * u <- we write it this way
  RealVectorValue vec(_u[_qp]*_u_vel[_qp],   // (U_k) * u_1
		      _u[_qp]*_v_vel[_qp],   // (U_k) * u_2
		      _u[_qp]*_w_vel[_qp]);  // (U_k) * u_3

  // (rho*u_k) * u + e_k * P [ e_k = unit vector in k-direction ]
  vec(_component) += _pressure[_qp];

  // -((rho*u_k) * u + e_k * P) * grad(phi)
  return -(vec*_grad_test[_i][_qp]);
}




Real
NSMomentumInviscidFlux::computeQpJacobian()
{
  RealVectorValue vec(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  
  // Note: Contribution to Jacobian due to P, which depends on U:
  //
  // P(U) = (gamma-1) * (U_4 - (1/2)*(U_1^2 + U_2^2 + U_3^3)/U_0^2)
  //
  // For the on-diagonal Jacobian, we need only differentiate P wrt U_i, i=1,2,3.
  // This gives:
  //
  // dP/dU_i = (1-gamma) * u_i
  //
  // where u_i is the i'th component of the velocity vector
  
  // The component'th entry of the on-diagonal Jacobian value is 2*u_i without the pressure
  // contribution.  Then the pressure adds (1-gamma)*u_i as noted above, so we end up
  // with (3-gamma)*u_i in the component'th position:
  vec(_component) = (3. - _gamma)*vec(_component);

  return - (vec * _grad_test[_i][_qp]) * _phi[_j][_qp];
}





Real
NSMomentumInviscidFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rho_var_number)
  {
    // std::cout << "Density: " << "_component=" << _component << ", jvar=" << jvar << std::endl;

    // Derivative of inviscid flux kernel wrt density:
    // x-mom: (-u_1^2 + (g-1)/2*V^2, -u_1*u_2            , -u_1*u_3            ) * grad(phi_i) * phi_j
    // y-mom: (-u_2*u_1            , -u_2^2 + (g-1)/2*V^2, -u_2*u_3            ) * grad(phi_i) * phi_j
    // z-mom: (-u_3*u_1            , -u_3*u_2            , -u_3^2 + (g-1)/2*V^2) * grad(phi_i) * phi_j
    //
    // In terms of the flux Jacobian entries from Ben's dissertation, these entries
    // come from the first column, in particular:
    //
    // x-mom: (A_1(2,1), A_2(2,1), A_3(2,1)) * grad(phi_i) * phi_j
    // y-mom: (A_1(3,1), A_2(3,1), A_3(3,1)) * grad(phi_i) * phi_j
    // z-mom: (A_1(4,1), A_2(4,1), A_3(4,1)) * grad(phi_i) * phi_j
    
    // Start with the velocity vector
    RealVectorValue vec(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    
    // Velocity vector magnitude, squared
    Real V2 = vec.size_sq();
    
    // Scale velocity vector by -1 * vec(_component)
    vec *= -vec(_component);

    // Add to the _component'th entry the quantity (gamma-1)/2 * V2
    vec(_component) += 0.5 * (_gamma-1.) * V2;

    // Return  -1 * (vec*grad(phi)) * phi_j
    return - (vec * _grad_test[_i][_qp]) * _phi[_j][_qp];
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

    // Create a vector according to the following three rules:
    RealVectorValue vec;
    
    // .) u_component is in entry jlocal
    vec(jlocal) = vel(_component);
    
    // .) (1-gamma)*u_jlocal is in entry _component
    vec(_component) = (1.-_gamma) * vel(jlocal);

    // .) 0 is in the remaining component
    // ...
    
    // Return -1*result * grad(phi_i) * phi_j
    return - (vec * _grad_test[_i][_qp]) * _phi[_j][_qp];
  }

  else if (jvar == _rhoe_var_number)
  {
    // std::cout << "Total energy: " << "_component=" << _component << ", jvar=" << jvar << std::endl;
    
    // The derivative of P(U) wrt to rho*E is simply (gamma-1), a constant
    RealVectorValue vec;
    vec(_component) = _gamma - 1.;

    // Return -1*result * grad(phi_i) * phi_j
    return - (vec * _grad_test[_i][_qp]) * _phi[_j][_qp];
  }

  // We shouldn't get here... jvar should have matched one of the if statements above!
  mooseError("computeQpOffDiagJacobian called with invalid jvar.");

  return 0;
}
