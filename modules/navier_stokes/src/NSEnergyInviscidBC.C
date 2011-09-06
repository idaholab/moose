#include "NSEnergyInviscidBC.h"

template<>
InputParameters validParams<NSEnergyInviscidBC>()
{
  InputParameters params = validParams<NSIntegratedBC>();

  // Required parameters
  params.addRequiredParam<Real>("specified_pressure", "The specified pressure for this boundary");

  return params;
}



NSEnergyInviscidBC::NSEnergyInviscidBC(const std::string & name, InputParameters parameters)
    : NSIntegratedBC(name, parameters),

      // Required parameters
      _specified_pressure(getParam<Real>("specified_pressure"))
{
}




Real NSEnergyInviscidBC::computeQpResidual()
{
  // n . (rho*H*u) v
  
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // rho*H*u = (rho*E + p)*u
  RealVectorValue conv_vec = (_rho_e[_qp] + _specified_pressure) * vel;

  // Dot with normal, hit with test function.
  return (conv_vec * _normals[_qp]) * _test[_i][_qp];
}




Real NSEnergyInviscidBC::computeQpJacobian()
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Derivative of convective term (above) wrt U_4
  Real conv_term = _phi[_j][_qp] * (vel * _normals[_qp]);

  return conv_term * _test[_i][_qp];
}




Real NSEnergyInviscidBC::computeQpOffDiagJacobian(unsigned jvar)
{
  // Convenience variables
  RealVectorValue U(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);

  // H_bar = E + p_bar/rho, the enthalpy evaluated at the specified pressure
  Real H_bar = (_rho_e[_qp] + _specified_pressure) / _rho[_qp];

  // Map jvar into the variable m for our problem, regardless of
  // how Moose has numbered things. 
  unsigned m = this->map_var_number(jvar);

  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // 
  // Convective term derivatives
  //

  // See notes for this term, specifically the specified-pressure subsonic outflow section
  Real conv_term = 0.;

  switch ( m )
  {
  case 0: // density
  {
    conv_term = -H_bar * (vel * _normals[_qp]) * _phi[_j][_qp];
    break;
  }

  case 1:
  case 2:
  case 3: // momentums
  {
    conv_term = H_bar * _normals[_qp](m-1) * _phi[_j][_qp];
    break;
  }

  case 4: // energy
    mooseError("Shouldn't be computing on-diagonal component!");

  default:
    mooseError("Shouldn't get here!");
  }

  // Finally, sum up the different contributions (with appropriate
  // sign) multiply by the test function, and return.
  return conv_term * _test[_i][_qp];
}
