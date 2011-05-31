#include "MomentumInviscidFlux.h"
 

template<>
InputParameters validParams<MomentumInviscidFlux>()
{
  InputParameters params = validParams<Kernel>();

  // Required parameters
  params.addRequiredParam<Real>("component", "");
  
  // Required copuled variables
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", "");
  params.addRequiredCoupledVar("pressure", "");
  return params;
}

MomentumInviscidFlux::MomentumInviscidFlux(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _u_vel_var(coupled("u")),
   _u_vel(coupledValue("u")),
   _v_vel_var(coupled("v")),
   _v_vel(coupledValue("v")),
   _w_vel_var(_dim == 3 ? coupled("w") : 0),
   _w_vel(_dim == 3 ? coupledValue("w") : _zero),
   _component(getParam<Real>("component")),
   _pressure_var(coupled("pressure")),
   _pressure(coupledValue("pressure"))
   //_pressure(getMaterialProperty<Real>("pressure"))// now an aux var
{
//  if(_component < 0)
//  {
//    std::cout<<"Must select a component for MomentumInviscidFlux"<<std::endl;
//    libmesh_error();
//  }
}

Real
MomentumInviscidFlux::computeQpResidual()
{
  // For _component = k,
  
  // (rho*U) * U_k = (rho*U_k) * U <- we write it this way
  RealVectorValue vec(_u[_qp]*_u_vel[_qp],   // (rho*U_k) * U_1
		      _u[_qp]*_v_vel[_qp],   // (rho*U_k) * U_2
		      _u[_qp]*_w_vel[_qp]);  // (rho*U_k) * U_3

  // (rho*U_k) * U + e_k * P [ e_k unit vector in k-direction ]
  vec(_component) += _pressure[_qp];

  // -((rho*U_k) * U + e_k * P) * grad(phi)
  return -(vec*_grad_test[_i][_qp]);
}




Real
MomentumInviscidFlux::computeQpJacobian()
{
  // FIXME: Not sure about these values, also, does not take into 
  // account that pressure P=P(U) and therefore should make some
  // contribution here as well...

  // For _component = k,
  // Derivative wrt (rho*U_k)
  RealVectorValue vec(_phi[_j][_qp]*_u_vel[_qp],
		      _phi[_j][_qp]*_v_vel[_qp],
		      _phi[_j][_qp]*_w_vel[_qp]);

  return -(vec*_grad_test[_i][_qp]);
}

Real
MomentumInviscidFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  // The value returned is the same, the value of jvar just determines the index
  Real val = _u[_qp] * _phi[_j][_qp];

  // Derivative wrt u_vel doesn't make sense... u_vel is not an independent variable
  if(jvar == _u_vel_var)
  {
    RealVectorValue vec(val,0,0);
    return -(vec*_grad_test[_i][_qp]);
  }
  else if(jvar == _v_vel_var)
  {
    RealVectorValue vec(0,val,0);
    return -(vec*_grad_test[_i][_qp]);
  }
  else if(jvar == _w_vel_var)
  {
    RealVectorValue vec(0,0,val);
    return -(vec*_grad_test[_i][_qp]);
  }

  // For all other variables, the derivative is zero...
  return 0;
}
