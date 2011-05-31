#include "MassInviscidFlux.h"

template<>
InputParameters validParams<MassInviscidFlux>()
{
  InputParameters params = validParams<Kernel>();
  
  // Required variables
  params.addRequiredCoupledVar("pu", "");
  params.addRequiredCoupledVar("pv", "");
  params.addCoupledVar("pw", "");
  
  // When computing the Jacobian, we need velocities alone.  Let's be 
  // consistent with other kernels that use Nodal Aux values for these
  // variables...
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", "");

  
  return params;
}

MassInviscidFlux::MassInviscidFlux(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
    _pu_var(coupled("pu")),
    _pu(coupledValue("pu")),
    _pv_var(coupled("pv")),
    _pv(coupledValue("pv")),
    _pw_var(_dim == 3 ? coupled("pw") : std::numeric_limits<unsigned int>::max()),
   _pw(_dim == 3 ? coupledValue("pw") : _zero),
    _u_vel_var(coupled("u")),
    _u_vel(coupledValue("u")),
    _v_vel_var(coupled("v")),
    _v_vel(coupledValue("v")),
    _w_vel_var(_dim == 3 ? coupled("w") : std::numeric_limits<unsigned int>::max()),
    _w_vel(_dim == 3 ? coupledValue("w") : _zero)
  {}



Real
MassInviscidFlux::computeQpResidual()
{
  // vec = rho*U
  RealVectorValue vec(_pu[_qp],_pv[_qp],_pw[_qp]);

  // -(rho*U) * grad(phi), negative sign comes from integration-by-parts
  return -(vec*_grad_test[_i][_qp]);
}



Real
MassInviscidFlux::computeQpJacobian()
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
MassInviscidFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _pu_var)
  {
    RealVectorValue vec(_phi[_j][_qp],0,0);
    return -(vec*_grad_test[_i][_qp]);
  }
  else if(jvar == _pv_var)
  {
    RealVectorValue vec(0,_phi[_j][_qp],0);
    return -(vec*_grad_test[_i][_qp]);
  }
  else if(jvar == _pw_var)
  {
    RealVectorValue vec(0,0,_phi[_j][_qp]);
    return -(vec*_grad_test[_i][_qp]);
  }

  return 0;
}
