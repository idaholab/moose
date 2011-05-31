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
   _dynamic_viscosity(getMaterialProperty<Real>("dynamic_viscosity")),
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
  
  // The thermal term is the only on-diagonal contribution
  // for this kernel... the tau*u term does not depend on U4!
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

  // Finally, we are ready to compute the thermal term: k * (vec1 + vec2) * grad(phi_i)
  Real thermal_term = k * (vec1 + vec2) * _grad_test[_i][_qp];
  

  // ...
  
  
  // Ready to compute the tau term

  // Kinematic viscosity, mu/rho
  Real mu = _dynamic_viscosity[_qp];
  Real nu = mu / U0;

  // So we don't get confused with zero vs. 1-based indexing...
  const unsigned x_component=0, y_component=1, z_component=2;

  // These convenience terms are the same as used in MomentumViscousFlux
  Real dU1dx = _grad_rho_u[_qp](x_component);
  Real dU1dy = _grad_rho_u[_qp](y_component);
  Real dU1dz = _grad_rho_u[_qp](z_component);

  Real dU2dx = _grad_rho_v[_qp](x_component);
  Real dU2dy = _grad_rho_v[_qp](y_component);
  Real dU2dz = _grad_rho_v[_qp](z_component);

  Real dU3dx = _grad_rho_w[_qp](x_component);
  Real dU3dy = _grad_rho_w[_qp](y_component);
  Real dU3dz = _grad_rho_w[_qp](z_component);
  
  Real X12 = dU1dy + dU2dx;
  Real X13 = dU1dz + dU3dx;
  Real X23 = dU2dz + dU3dy;

  Real drhodx = _grad_rho[_qp](x_component);
  Real drhody = _grad_rho[_qp](y_component);
  Real drhodz = _grad_rho[_qp](z_component);

  Real dphijdx = _grad_phi[_j][_qp](x_component);
  Real dphijdy = _grad_phi[_j][_qp](y_component);
  Real dphijdz = _grad_phi[_j][_qp](z_component);

  // The momentum vector
  RealVectorValue U(U1, U2, U3);
  
  // The vector (U1*dU1dx, U2*dU2dy, U3*dU3dz)
  RealVectorValue UdivU(U1*dU1dx, U2*dU2dy, U3*dU3dz);

  // The scalar value divU = dU1dx + dU2dy + dU2dz
  Real divU = dU1dx + dU2dy + dU3dz;

  // The components of the divergence, stored in a vector
  RealVectorValue divU_vector(dU1dx, dU2dy, dU3dz);

  // The momentum vector dotted with the ith test function gradient
  Real U_dot_grad_phi_i = U * _grad_test[_i][_qp];

  // The momentum vector dotted with the jth trial function gradient
  Real U_dot_grad_phi_j = U * _grad_phi[_j][_qp];

  // The jth component of the trial function dotted with the ith component of the test
  Real grad_phi_j_dot_grad_phi_i = _grad_phi[_j][_qp]*_grad_test[_i][_qp];

  // The momentum vector dotted with the density gradient
  Real U_dot_grad_U0 = U * _grad_rho[_qp];

  // Gradient of rho dotted with ith component of test function
  Real grad_U0_dot_grad_phi_i = _grad_rho[_qp] * _grad_test[_i][_qp];

  // ...

  // Contribution due to tau term
  Real viscous_term = 0.;

  // Derivative wrt rho
  if (jvar == _rho_var_number)
  {
    // viscous_vec1 to be dotted with grad(phi_i)
    RealVectorValue viscous_vec1(U2*X12+U3*X13, 
				 U1*X12+U3*X23, 
				 U1*X13+U2*X23);
    viscous_vec1 *= -2.*mu*phij/U03;

    Real viscous_term1 = 
      -(mu*mom2/U03)         * grad_phi_j_dot_grad_phi_i
      -(4.*mu/U03)*phij      * (UdivU*_grad_test[_i][_qp])
      +(3.*mu*mom2/U04)*phij * (grad_U0_dot_grad_phi_i);

    Real viscous_term2 = 
      (+(4./3.*mu*divU/U03)*phij 
       -(mu/3./U03)              * (U_dot_grad_phi_j)
       +(mu/U04)*phij            * (U_dot_grad_U0   )) * U_dot_grad_phi_i;
  
    // Compute the viscous term
    viscous_term = (viscous_vec1*_grad_test[_i][_qp]) + viscous_term1 + viscous_term2;
  }

  
  else if ((jvar == _rhou_var_number) || (jvar == _rhov_var_number) || (jvar == _rhow_var_number))
  {
    // Map jvar into beta = {0,1,2}, regardless of how Moose has numbered things.
    unsigned beta = 0;

    // Vector of "X" off-diagonal cross terms, depends on jvar
    RealVectorValue X(0., X12, X13);
    
    if (jvar == _rhov_var_number)
    {
      beta = 1;
      X = RealVectorValue(X12, 0., X23);
    }
    else if (jvar == _rhow_var_number)
    {
      beta = 2;
      X = RealVectorValue(X13, X23, 0.);
    }

    viscous_term += 
      -((2.*mu/3./U02)*_grad_phi[_j][_qp](beta) 
	+ phij * nu * _grad_rho[_qp](beta) / 3. / U02) * U_dot_grad_phi_i;

    viscous_term +=
      ((mu/U02)*U_dot_grad_phi_j 
       - nu*phij*U_dot_grad_U0 / 3. / U02 
       + (2.*mu*divU_vector(beta) - 2./3.*mu*divU)*phij/U02) * _grad_test[_i][_qp](beta);

    viscous_term += 
      (mu/U02) * phij * (X*_grad_test[_i][_qp]);
      
    viscous_term += 
      (mu*U(beta)/U02) * grad_phi_j_dot_grad_phi_i;
    
    viscous_term += 
      (-2*nu*U(beta)*phij/U02) * grad_U0_dot_grad_phi_i;
  }

  // Return the combined thermal and viscous term contributions for this off-diagonal contribution
  return thermal_term + viscous_term;
}
