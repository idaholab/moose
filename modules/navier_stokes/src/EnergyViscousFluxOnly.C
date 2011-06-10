#include "EnergyViscousFluxOnly.h"
 

template<>
InputParameters validParams<EnergyViscousFluxOnly>()
{
  InputParameters params = validParams<Kernel>();
  
  // Declare some required coupled variables
  // Velocities likely to be AuxVariables, but it doesn't matter,
  // they can be coupled just the same...
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // only required in 3D

  // Required copuled variables for Jacobian terms
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "x-momentum");
  params.addRequiredCoupledVar("rhov", "y-momentum");
  params.addCoupledVar("rhow", "z-momentum"); // only required in 3D

  return params;
}




EnergyViscousFluxOnly::EnergyViscousFluxOnly(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _u_vel(coupledValue("u")),
   _v_vel(coupledValue("v")),
   _w_vel(_dim == 3 ? coupledValue("w") : _zero),
   _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor")),
   _dynamic_viscosity(getMaterialProperty<Real>("dynamic_viscosity")),
   _rho_var_number( coupled("rho") ),
   _rhou_var_number( coupled("rhou") ),
   _rhov_var_number( coupled("rhov") ),
   _rhow_var_number( _dim == 3 ? coupled("rhow") : libMesh::invalid_uint),
   _rho(coupledValue("rho")),
   _rho_u(coupledValue("rhou")),
   _rho_v(coupledValue("rhov")),
   _rho_w( _dim == 3 ? coupledValue("rhow") : _zero),
   _grad_rho(coupledGradient("rho")),
   _grad_rho_u(coupledGradient("rhou")),
   _grad_rho_v(coupledGradient("rhov")),
   _grad_rho_w( _dim == 3 ? coupledGradient("rhow") : _grad_zero)
  {}




Real
EnergyViscousFluxOnly::computeQpResidual()
{
  // (tau * u) * grad(phi)
  RealVectorValue velocity(_u_vel[_qp],_v_vel[_qp],_w_vel[_qp]);
  RealVectorValue vec = _viscous_stress_tensor[_qp] * velocity;

  return vec * _grad_test[_i][_qp];
}




Real
EnergyViscousFluxOnly::computeQpJacobian()
{
  // No dependence of this term on U_4!
  return 0.;
}



Real
EnergyViscousFluxOnly::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Convenience variables

  Real U0 = _rho[_qp];
  Real U1 = _rho_u[_qp];
  Real U2 = _rho_v[_qp];
  Real U3 = _rho_w[_qp];
  
  Real mom2 = U1*U1 + U2*U2 + U3*U3;
  Real U02 = U0*U0;  // rho^2
  Real U03 = U02*U0; // rho^3
  Real U04 = U03*U0; // rho^4
  
  Real phij = _phi[_j][_qp];

  // ...

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
  return viscous_term;
}
