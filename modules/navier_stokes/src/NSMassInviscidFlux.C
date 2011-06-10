#include "NSMassInviscidFlux.h"

template<>
InputParameters validParams<NSMassInviscidFlux>()
{
  InputParameters params = validParams<Kernel>();
  
  // Required variables
  params.addRequiredCoupledVar("rhou", "");
  params.addRequiredCoupledVar("rhov", "");
  params.addCoupledVar("rhow", "");
  
  // When computing the Jacobian, we need velocities alone.  Let's be 
  // consistent with other kernels that use Nodal Aux values for these
  // variables...
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", "");

  
  return params;
}

NSMassInviscidFlux::NSMassInviscidFlux(const std::string & name, InputParameters parameters)
    :Kernel(name, parameters),
     _rhou_var_number(coupled("rhou")),
     _rhou(coupledValue("rhou")),
     _rhov_var_number(coupled("rhov")),
     _rhov(coupledValue("rhov")),
     _rhow_var_number(_dim == 3 ? coupled("rhow") : std::numeric_limits<unsigned int>::max()),
     _rhow(_dim == 3 ? coupledValue("rhow") : _zero),
     _u_vel_var(coupled("u")),
     _u_vel(coupledValue("u")),
     _v_vel_var(coupled("v")),
     _v_vel(coupledValue("v")),
     _w_vel_var(_dim == 3 ? coupled("w") : std::numeric_limits<unsigned int>::max()),
     _w_vel(_dim == 3 ? coupledValue("w") : _zero)
{}



Real
NSMassInviscidFlux::computeQpResidual()
{
  // vec = rho*U
  RealVectorValue vec(_rhou[_qp],_rhov[_qp],_rhow[_qp]);

  // -(rho*U) * grad(phi), negative sign comes from integration-by-parts
  return -(vec*_grad_test[_i][_qp]);
}



Real
NSMassInviscidFlux::computeQpJacobian()
{
  // This seems weird at first glance, but remember we have to differentiate
  // wrt the *conserved* variables 
  //
  // [ U_0 ] = [ rho       ]
  // [ U_1 ] = [ rho * u_1 ]
  // [ U_2 ] = [ rho * u_2 ]
  // [ U_3 ] = [ rho * u_3 ]
  // [ U_4 ] = [ rho * E   ] 
  //
  // and the inviscid mass flux residual, in terms of these variables, is:
  //
  // f(U) = ( U_k * dphi_i/dx_k ), summation over k=1,2,3
  //
  // ie. does not depend on U_0, the on-diagonal Jacobian component.
  return 0.;
}



Real
NSMassInviscidFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _rhou_var_number)
  {
    RealVectorValue vec(_phi[_j][_qp],0,0);
    return -(vec*_grad_test[_i][_qp]);
  }
  else if(jvar == _rhov_var_number)
  {
    RealVectorValue vec(0,_phi[_j][_qp],0);
    return -(vec*_grad_test[_i][_qp]);
  }
  else if(jvar == _rhow_var_number)
  {
    RealVectorValue vec(0,0,_phi[_j][_qp]);
    return -(vec*_grad_test[_i][_qp]);
  }

  return 0;
}
