#include "EnergyViscousFlux.h"
 

template<>
InputParameters validParams<EnergyViscousFlux>()
{
  InputParameters params = validParams<Kernel>();
  
  // Declare some required coupled variables
  // Velocities likely to be AuxVariables, but it doesn't matter,
  // they can be coupled just the same...
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // only required in 3D
  params.addRequiredCoupledVar("temperature", "");

  // Required copuled variables for Jacobian terms
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "x-momentum");
  params.addRequiredCoupledVar("rhov", "y-momentum");
  params.addCoupledVar("rhow", "z-momentum"); // only required in 3D
  params.addRequiredCoupledVar("rhoe", "total energy");

  // Parameters with default values
  params.addRequiredParam<Real>("cv", "Specific heat at constant volume");

  return params;
}




EnergyViscousFlux::EnergyViscousFlux(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _u_vel(coupledValue("u")),
   _v_vel(coupledValue("v")),
   _w_vel(_dim == 3 ? coupledValue("w") : _zero),
   _temp_var(coupled("temperature")),
   _grad_temp(coupledGradient("temperature")),
   _temp(coupledValue("temperature")),
   _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor")),
   _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),
   _rho_var_number( coupled("rho") ),
   _rhou_var_number( coupled("rhou") ),
   _rhov_var_number( coupled("rhov") ),
   _rhow_var_number( _dim == 3 ? coupled("rhow") : libMesh::invalid_uint),
   _rhoe_var_number( coupled("rhoe") ),
   _rho(coupledValue("rho")),
   _rho_u(coupledValue("rhou")),
   _rho_v(coupledValue("rhov")),
   _rho_w( _dim == 3 ? coupledValue("rhow") : _zero),
   _rho_e(coupledValue("rhoe")),
   _grad_rho(coupledGradient("rho")),
   _grad_rho_u(coupledGradient("rhou")),
   _grad_rho_v(coupledGradient("rhov")),
   _grad_rho_w( _dim == 3 ? coupledGradient("rhow") : _grad_zero),
   _grad_rho_e(coupledGradient("rhoe")),
   _cv(getParam<Real>("cv"))
  {}




Real
EnergyViscousFlux::computeQpResidual()
{
  // (k*grad(T) + tau * u) * grad(phi)
  RealVectorValue velocity(_u_vel[_qp],_v_vel[_qp],_w_vel[_qp]);

  
  // Debugging, don't really need T just grad(T)
  //std::cout << "_u_vel=" << _u_vel[_qp] << std::endl;
  //std::cout << "_temp=" << _temp[_qp] << std::endl;

  // vec = grad(T)
  RealVectorValue vec = _grad_temp[_qp];
  //std::cout << vec << std::endl;

  // vec = k*grad(T)
  vec *= _thermal_conductivity[_qp];

  // vec = tau*u + k*grad(T)
  vec += _viscous_stress_tensor[_qp] * velocity;

  return vec * _grad_test[_i][_qp];
}




Real
EnergyViscousFlux::computeQpJacobian()
{
  Real k = _thermal_conductivity[_qp];

  Real rho  = _rho[_qp];
  Real rho2 = rho*rho; // rho^2
  Real phij = _phi[_j][_qp];

  RealVectorValue vec1 = -(phij/rho2/_cv)*_grad_rho[_qp];
  RealVectorValue vec2 =  (1./rho/_cv)*_grad_phi[_j][_qp];
  
  return k * (vec1 + vec2) * _grad_test[_i][_qp];
}



Real
EnergyViscousFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Convenience variables
  Real k = _thermal_conductivity[_qp];

  Real U0 = _rho[_qp];
  Real U1 = _rho_u[_qp];
  Real U2 = _rho_v[_qp];
  Real U3 = _rho_w[_qp];
  Real U4 = _rho_e[_qp];
  
  Real mom2 = U1*U1 + U2*U2 + U3*U3;
  Real U02 = U0*U0;  // rho^2
  Real U03 = U02*U0; // rho^3
  Real U04 = U03*U0; // rho^4
  
  Real phij = _phi[_j][_qp];

  // ...
  
  // The derivative of the temperature aux variable wrt all the conserved variables.
  // This was computed in Maple.  Only one of these actual values will be used, but
  // it is easier to organize the code this way and I doubt it makes any difference
  // to performance.
  std::vector<Real> dTdU(5);
  dTdU[0] = -U4/U02/_cv + mom2/U03/_cv; // derivative wrt U0=rho
  dTdU[1] = -U1/U02/_cv;                // derivative wrt U1
  dTdU[2] = -U2/U02/_cv;                // derivative wrt U2
  dTdU[3] = -U3/U02/_cv;                // derivative wrt U3
  dTdU[4] = 1./U0/_cv;                  // derivative wrt U4, (not used since this is off-diagonal)
  
  
  // The derivative of dTdU wrt variable (U_k) FE component j.
  // The values of this depend on jvar and are filled in below...
  std::vector<Real> dTdU_dUk(5);

  RealVectorValue vec1;

  // Fill up the dTdU_dUk vector depending on jvar
  if (jvar == _rho_var_number)
  {
    dTdU_dUk[0] = (2.*U4/_cv/U03 - 3.*mom2/_cv/U04 )*phij;
    dTdU_dUk[1] = 2.*phij*U1/U03/_cv;
    dTdU_dUk[2] = 2.*phij*U2/U03/_cv;
    dTdU_dUk[3] = 2.*phij*U3/U03/_cv;
    dTdU_dUk[4] = -phij/U02/_cv;

    // Compute the "dot product" dphij/dx * dT/dU but this is 
    // only a single entry
    vec1 = _grad_phi[_j][_qp] * dTdU[0];
  }

  else if (jvar == _rhou_var_number)
  {
    dTdU_dUk[0] = 2.*phij*U1/U03/_cv;
    dTdU_dUk[1] = -phij/U02/_cv;
    
    // other entries of dTdU_dUk are zero...

    // Compute the "dot product" dphij/dx * dT/dU but this is 
    // only a single entry
    vec1 = _grad_phi[_j][_qp] * dTdU[1];
  }

  else if (jvar == _rhov_var_number)
  {
    dTdU_dUk[0] = 2.*phij*U2/U03/_cv;
    dTdU_dUk[2] = -phij/U02/_cv;
    
    // other entries of dTdU_dUk are zero...

    // Compute the "dot product" dphij/dx * dT/dU but this is 
    // only a single entry
    vec1 = _grad_phi[_j][_qp] * dTdU[2];
  }

  else if (jvar == _rhow_var_number)
  {
    dTdU_dUk[0] = 2.*phij*U3/U03/_cv;
    dTdU_dUk[3] = -phij/U02/_cv;
    
    // other entries of dTdU_dUk are zero...

    // Compute the "dot product" dphij/dx * dT/dU but this is 
    // only a single entry
    vec1 = _grad_phi[_j][_qp] * dTdU[3];
  }

  else if (jvar == _rhoe_var_number)
  {
    mooseError("This is supposed to be computing off-diagonal Jacobian entries1");
    
    // dTdU_dUk[0] = -phij/U02/_cv;
    // 
    // // other entries of dTdU_dUk are zero...
    // 
    // // Compute the "dot product" dphij/dx * dT/dU but this is 
    // // only a single entry
    // vec1 = _grad_phi[_j][_qp] * dTdU[4];
  }


  // Compute the vector:
  // (dU/dx \cdot dTdU_dUk, 
  //  dU/dy \cdot dTdU_dUk, 
  //  dU/dz \cdot dTdU_dUk)
  //
  // where U = (U0, U1, U2, U3, U4) is the vector of all conserved
  // vars, and dU/dx = (dU0/dx, dU1/dx, dU2/dx, dU3/dx, dU4/dx),
  // for example.
  RealVectorValue vec2;
  for (unsigned i=0; i<3; ++i)
  {
    vec2(i) = 
      _grad_rho[_qp](i)*dTdU_dUk[0]  +
      _grad_rho_u[_qp](i)*dTdU_dUk[1] + 
      _grad_rho_v[_qp](i)*dTdU_dUk[2] + 
      _grad_rho_w[_qp](i)*dTdU_dUk[3] + 
      _grad_rho_e[_qp](i)*dTdU_dUk[4];
  }

  // Finally, we are ready to return: k * (vec1 + vec2) * grad(phi_i)
  return k * (vec1 + vec2) * _grad_test[_i][_qp];
}
