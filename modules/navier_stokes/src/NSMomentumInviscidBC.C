#include "NSMomentumInviscidBC.h"

template<>
InputParameters validParams<NSMomentumInviscidBC>()
{
  InputParameters params = validParams<NSIntegratedBC>();
  
  // TODO: We might want to actually couple with the pressure aux var?
  // params.addRequiredCoupledVar("pressure", "");

  // Required parameters
  params.addRequiredParam<unsigned>("component", "(0,1,2) = (x,y,z) for which momentum component this BC is applied to");
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats.");
  // params.addRequiredParam<Real>("specified_pressure", "The specified pressure for this boundary");
  
  return params;
}




NSMomentumInviscidBC::NSMomentumInviscidBC(const std::string & name, InputParameters parameters)
    : NSIntegratedBC(name, parameters),
      //_pressure(coupledValue("pressure")),

      // Parameters to be specified in input file block...
      _component(getParam<unsigned>("component")),
      _gamma(getParam<Real>("gamma")),
      // _specified_pressure(getParam<Real>("specified_pressure")),

      // Object for computing deriviatives of pressure
      _pressure_derivs(*this)
{
}



// Real NSMomentumInviscidBC::computeQpResidual()
// {
//   // n . (rho*uu + Ip) . v
//   
//   // Velocity vector object
//   RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
// 
//   // Vector-valued test function
//   RealVectorValue v_test;
//   v_test(_component) = _test[_i][_qp];
// 
//   // The outer product matrix "uu", constructed by rows
//   RealTensorValue uu(_u_vel[_qp]*vel,
// 		     _v_vel[_qp]*vel,
// 		     _w_vel[_qp]*vel);
// 
//   // The "inviscid" contribution: n . rho*uu . v
//   Real conv_term = _rho[_qp] * (_normals[_qp] * (uu * v_test));
// 
//   // The pressure contribution: p * n(component) * phi_i
//   Real press_term = _specified_pressure * _normals[_qp](_component) * _test[_i][_qp];
// 
//   // Sum up contributions and return
//   return conv_term + press_term;
// }



// Real NSMomentumInviscidBC::computeQpJacobian()
// {
//   // Velocity vector object
//   RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
// 
//   // See Eqn. (69) from the notes for the inviscid boundary terms
//   Real conv_term = ((vel * _normals[_qp]) + vel(_component)*_normals[_qp](_component)) * _phi[_j][_qp] * _test[_i][_qp];  
// 
//   // Return the result.  We could return it directly but this is
//   // convenient for printing...
//   return conv_term;
// }



// Real NSMomentumInviscidBC::computeQpOffDiagJacobian(unsigned jvar)
// {
//   // See Eqns. (33), (34), and (35) from the notes for the inviscid
//   // boundary terms (Be careful, they also include pressure
//   // contributions which we do not need here!)
// 
//   // Map jvar into the variable m for our problem, regardless of
//   // how Moose has numbered things. 
//   unsigned m = this->map_var_number(jvar);
// 
//   // Velocity vector object
//   RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
// 
//   // Variable to store convective contribution to boundary integral.
//   Real conv_term = 0.;
// 
//   // Inviscid components
//   switch ( m )
//   {
// 
//   case 0: // density
//   {
//     // Note: the minus sign here is correct, it comes from differentiating wrt U_0
//     // (rho) which is in the denominator.
//     conv_term = -vel(_component) * (vel * _normals[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
//     break;
//   }
// 
//   case 1:
//   case 2:
//   case 3:  // momentums
//   {
//     if (m-1 == _component)
//     {
//       // on-diagonal - we should not be computing on-diagonal Jacobians here
//       mooseError("Invalid jvar!");
//     }
//     else
//     {
//       // off-diagonal
//       conv_term = vel(_component) * _normals[_qp](m-1) * _phi[_j][_qp] * _test[_i][_qp];
//     }
//     break;
//   }
// 
//   case 4: // energy
//   {
//     // Only the pressure term would have an energy Jacobian, but the pressure is assumed known
//     // in this BC.
//     conv_term = 0;
//     break;
//   }
// 
//   default:
//     mooseError("Shouldn't get here!");
//   }
// 
//   // Return the result.  We could return it directly but this is
//   // convenient for printing...
//   return conv_term;
// }





Real NSMomentumInviscidBC::pressure_qp_residual(Real pressure)
{
  // n . (Ip) . v
  
  // The pressure contribution: p * n(component) * phi_i
  Real press_term = pressure * _normals[_qp](_component) * _test[_i][_qp];

  // Return value, or print it first if debugging...
  return press_term;
}





Real NSMomentumInviscidBC::pressure_qp_jacobian(unsigned var_number)
{
  return _normals[_qp](_component) * _pressure_derivs.get_grad(var_number) * _phi[_j][_qp] * _test[_i][_qp];  
}




Real NSMomentumInviscidBC::convective_qp_residual(RealVectorValue rhou_udotn)
{
  // n . (rho*uu) . v = rho*(u.n)*(u.v) = (rho*u)(u.n) . v
  
  // Vector-valued test function
  RealVectorValue v_test;
  v_test(_component) = _test[_i][_qp];

  // The "inviscid" contribution: (rho*u)(u.n) . v
  Real conv_term = rhou_udotn * v_test;

  // Return value, or print it first if debugging...
  return conv_term;
}




Real NSMomentumInviscidBC::convective_qp_jacobian(unsigned var_number)
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Variable to store convective contribution to boundary integral.
  Real conv_term = 0.;

  // Inviscid components
  switch ( var_number )
  {

  case 0: // density
  {
    // Note: the minus sign here is correct, it comes from differentiating wrt U_0
    // (rho) which is in the denominator.
    conv_term = -vel(_component) * (vel * _normals[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
    break;
  }

  case 1:
  case 2:
  case 3:  // momentums
  {
    if (var_number-1 == _component)
    {
      // See Eqn. (68) from the notes for the inviscid boundary terms
      conv_term = ((vel * _normals[_qp]) + vel(_component)*_normals[_qp](_component)) * _phi[_j][_qp] * _test[_i][_qp];  
    }
    else
    {
      // off-diagonal
      conv_term = vel(_component) * _normals[_qp](var_number-1) * _phi[_j][_qp] * _test[_i][_qp];
    }
    break;
  }

  case 4: // energy
  {
    // No derivative wrt energy
    conv_term = 0.;
    break;
  }

  default:
    mooseError("Shouldn't get here!");
  }

  // Return the result.  We could return it directly from the switch statement, but this is
  // convenient for printing...
  return conv_term;
}
