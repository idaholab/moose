#include "EnergyInviscidFlux.h"
 
template<>
InputParameters validParams<EnergyInviscidFlux>()
{
  InputParameters params = validParams<Kernel>();

  // Required coupled nodal aux variables
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", "");
  params.addRequiredCoupledVar("pressure", ""); // Now computed as an AuxKernel

  // Required parameters
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats");

  // Required "coupled" variables, only the numerical index of these
  // variables is required, not their actual value.
  params.addRequiredCoupledVar("rho", "");
  params.addRequiredCoupledVar("rhou", "");
  params.addRequiredCoupledVar("rhov", "");
  params.addCoupledVar("rhow", ""); // only required in 3D
  
  // Require this *or* rho, but not both?
  params.addRequiredCoupledVar("enthalpy", "");

  return params;
}

EnergyInviscidFlux::EnergyInviscidFlux(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _u_vel(coupledValue("u")),
   _v_vel(coupledValue("v")),
   _w_vel(_dim == 3 ? coupledValue("w") : _zero),
   _pressure(coupledValue("pressure")),
   _gamma(getParam<Real>("gamma")),
   _rho_var_number( coupled("rho") ),
   _rhou_var_number( coupled("rhou") ),
   _rhov_var_number( coupled("rhov") ),
   _rhow_var_number( _dim == 3 ? coupled("rhow") : libMesh::invalid_uint),
   _rho(coupledValue("rho")),
   _enthalpy(coupledValue("enthalpy"))
{}




Real
EnergyInviscidFlux::computeQpResidual()
{
  // H = total enthalpy = E + P/rho
  // => rho * u * H = rho * u ( E + P/rho)
  //                =       u ( rho*E + P)

  // velocity vector
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  
  // Multiply vector U by the scalar value (rho*E + P) to get rho * U * H
  // vel *= (_u[_qp] + _pressure[_qp]);

  // Multiply velocity vector by the scalar (rho * H)
  vel *= (_rho[_qp] * _enthalpy[_qp]);

  // Return -1 * vel * grad(phi_i)
  return -(vel*_grad_test[_i][_qp]);
}




Real
EnergyInviscidFlux::computeQpJacobian()
{
  // Derivative of this kernel wrt rho*E
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // -gamma * phi_j * (U*grad(phi_i))
  return -_gamma * _phi[_j][_qp] * (vel*_grad_test[_i][_qp]);
}




Real
EnergyInviscidFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real V2 = vel.size_sq();

  // TODO, make enthalpy a nodal aux?
  // Real total_enthalpy = (_u[_qp] + _pressure[_qp]) / _p[_qp];
  
  // Derivative wrt density
  if (jvar == _rho_var_number)
  {
    return -((0.5*(_gamma-1)*V2 - _enthalpy[_qp]) * _phi[_j][_qp] * (vel * _grad_test[_i][_qp]));
  }
  
  // Derivatives wrt momentums
  else if ((jvar == _rhou_var_number) || (jvar == _rhov_var_number) || (jvar == _rhow_var_number))
  {
    // Map jvar into jlocal = {0,1,2}, regardless of how Moose has numbered things.
    unsigned jlocal = 0;
    
    if (jvar == _rhov_var_number)
      jlocal = 1;
    else if (jvar == _rhow_var_number)
      jlocal = 2;

    // Scale the velocity vector by the scalar (1-gamma)*vel(jlocal)
    vel *= (1.-_gamma)*vel(jlocal);

    // Add in the enthalpy in the jlocal'th entry
    vel(jlocal) += _enthalpy[_qp];

    // Return -1 * (vel * grad(phi_i)) * phi_j
    return -(vel*_grad_test[_i][_qp]) * _phi[_j][_qp];
  }

  // We shouldn't get here... jvar should have matched one of the if statements above!
  return 0;
}

