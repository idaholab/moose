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
   _component(getParam<unsigned>("component")),
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
  // The on-diagonal entry corresponds to variable number _component+1.
  return this->compute_jacobian(_component+1); 
}





Real
NSMomentumInviscidFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Map jvar into the variable m for our problem, regardless of
  // how Moose has numbered things. 
  unsigned m = 99;
 
  if (jvar == _rho_var_number)
    m = 0;
  else if (jvar == _rhou_var_number)
    m = 1;
  else if (jvar == _rhov_var_number)
    m = 2;
  else if (jvar == _rhow_var_number)
    m = 3;
  else if (jvar == _rhoe_var_number)
    m = 4;
  else
    mooseError("Invalid jvar!");

  return this->compute_jacobian(m);
}



Real NSMomentumInviscidFlux::compute_jacobian(unsigned m)
{
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  switch ( m )
  {
  case 0: // density
  {
    Real V2 = vel.size_sq();

    return vel(_component) * (vel*_grad_test[_i][_qp]) - 0.5 * (_gamma-1.) * V2 * _grad_test[_i][_qp](_component);
  }

  case 1:
  case 2:
  case 3: // momentums
  {
    // Map m into m_local = {0,1,2}
    unsigned m_local = m - 1;

    // Kronecker delta
    Real delta_kl = (_component == m_local ? 1. : 0.);

    return -1. * (vel(_component) * _grad_test[_i][_qp](m_local)
		  + delta_kl * (vel * _grad_test[_i][_qp])
		  + (1.-_gamma) * vel(m_local) *_grad_test[_i][_qp](_component)) * _phi[_j][_qp];
  }

  case 4: // energy
    return -1. * (_gamma - 1.) * _phi[_j][_qp] * _grad_test[_i][_qp](_component);

  default:
    mooseError("Shouldn't get here!");
  }

  // Won't get here!
  return 0;

}
