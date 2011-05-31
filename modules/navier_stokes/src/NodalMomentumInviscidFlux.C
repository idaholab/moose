#include "NodalMomentumInviscidFlux.h"

template<>
InputParameters validParams<NodalMomentumInviscidFlux>()
{
  InputParameters params = validParams<Kernel>();

  // Required copuled variables
  params.addRequiredCoupledVar("F1", "");
  params.addRequiredCoupledVar("F2", "");
  params.addCoupledVar("F3", ""); // only required in 3D

  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // only required in 3D

  // Required parameters
  params.addRequiredParam<int>("component", "");
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats");
  
  return params;
}




NodalMomentumInviscidFlux::NodalMomentumInviscidFlux(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _F1(coupledValue("F1")),
   _F2(coupledValue("F2")),
   _F3(_dim == 3 ? coupledValue("F3") : _zero),
   _u_vel(coupledValue("u")),
   _v_vel(coupledValue("v")),
   _w_vel(_dim == 3 ? coupledValue("w") : _zero),
   _component(getParam<int>("component")),
   _gamma(getParam<Real>("gamma"))
{
}




Real
NodalMomentumInviscidFlux::computeQpResidual()
{
  // Build vector out of nodal aux values already computed
  RealVectorValue vec(_F1[_qp], _F2[_qp], _F3[_qp]);

  // Dot with gradient of test function
  return -(vec*_grad_test[_i][_qp]);
}


Real
NodalMomentumInviscidFlux::computeQpJacobian()
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
NodalMomentumInviscidFlux::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{

// Not yet implemented
//  // The value returned is the same, the value of jvar just determines the index
//  Real val = _u[_qp] * _phi[_j][_qp];
//
//  // Derivative wrt u_vel doesn't make sense... u_vel is not an independent variable
//  if(jvar == _u_vel_var)
//  {
//    RealVectorValue vec(val,0,0);
//    return -(vec*_grad_test[_i][_qp]);
//  }
//  else if(jvar == _v_vel_var)
//  {
//    RealVectorValue vec(0,val,0);
//    return -(vec*_grad_test[_i][_qp]);
//  }
//  else if(jvar == _w_vel_var)
//  {
//    RealVectorValue vec(0,0,val);
//    return -(vec*_grad_test[_i][_qp]);
//  }

  // For all other variables, the derivative is zero...
  return 0;
}
