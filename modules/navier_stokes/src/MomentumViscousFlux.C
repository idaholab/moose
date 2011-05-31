#include "MomentumViscousFlux.h"
 

template<>
InputParameters validParams<MomentumViscousFlux>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<Kernel>();

  // component is a required parameter, so make it so!
  params.addRequiredParam<unsigned>("component", "");

  // Required copuled variables for Jacobian terms
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "x-momentum");
  params.addRequiredCoupledVar("rhov", "y-momentum");
  params.addCoupledVar("rhow", "z-momentum"); // only required in 3D

  return params;
}




MomentumViscousFlux::MomentumViscousFlux(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _component(getParam<unsigned>("component")),
   _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor")),
   _dynamic_viscosity(getMaterialProperty<Real>("dynamic_viscosity")),
   _rho(coupledValue("rho")),
   _grad_rho(coupledGradient("rho")),
   _rho_var_number( coupled("rho") ),
   _rhou_var_number( coupled("rhou") ),
   _rhov_var_number( coupled("rhov") ),
   _rhow_var_number( _dim == 3 ? coupled("rhow") : libMesh::invalid_uint),
   _rho_u(coupledValue("rhou")),
   _grad_rho_u(coupledGradient("rhou")),
   _rho_v(coupledValue("rhov")),
   _grad_rho_v(coupledGradient("rhov")),
   _rho_w( _dim == 3 ? coupledValue("rhow") : _zero),
   _grad_rho_w( _dim == 3 ? coupledGradient("rhow") : _grad_zero)
{
  if(_component > 2)
  {
    std::cout << "Must select a component<=2 for MomentumViscousFlux" << std::endl;
    libmesh_error();
  }
}




Real
MomentumViscousFlux::computeQpResidual()
{
  // Yay for less typing!
  RealTensorValue & vst = _viscous_stress_tensor[_qp];
  
  // _component'th column of vst...
  RealVectorValue vec(vst(0,_component),vst(1,_component),vst(2,_component));

  // ... dotted with grad(phi), note: sign is positive as this term was -div(tau) on the lhs
  return vec*_grad_test[_i][_qp];
}




Real
MomentumViscousFlux::computeQpJacobian()
{
  // We could do everything in one return statement, but for the sake of readability,
  // let's break it up into its logical parts.
  // return (nu * _grad_phi[_j][_qp] - (nu/_rho[_qp])*_phi[_j][_qp]*_grad_rho[_qp])*_grad_test[_i][_qp] 
  //   + (1./3.)*(nu*_grad_phi[_j][_qp](_component) - (nu/_rho[_qp])*_phi[_j][_qp]*_grad_rho[_qp](_component))*_grad_test[_i][_qp](_component);
  
  // Kinematic viscosity, mu/rho
  Real nu = _dynamic_viscosity[_qp] / _rho[_qp];

  // Component'th entry of grad(phi_i)
  Real dphii_dxc = _grad_test[_i][_qp](_component);

  // Component'th entry of grad(phi_j)
  Real dphij_dxc = _grad_phi[_j][_qp](_component);
  
  // Component'th entry of grad(rho)
  Real drho_dxc = _grad_rho[_qp](_component);
  
  // ...

  // This is your "normal" grad(phi_i)*grad(phi_j) diffusion-type term
  Real val1 = nu * (_grad_phi[_j][_qp] * _grad_test[_i][_qp]);

  // This term is proportional to grad(rho), and arises from differencing
  // wrt to conserved variables.
  Real val2 = (nu/_rho[_qp]) * _phi[_j][_qp] * (_grad_rho[_qp]*_grad_test[_i][_qp]);

  // This term is similar to the first, but only contributes to the _component'th entry of grad(phi_i) 
  Real val3 = (1./3.) * nu * dphij_dxc * dphii_dxc;

  // This term is similar to the second, but only contributes to the _component'th entry of grad(phi_i)
  Real val4 = (1./3.) * (nu/_rho[_qp]) * _phi[_j][_qp] * drho_dxc * dphii_dxc;

  return (val1 + val3) - (val2 + val4);
}




Real
MomentumViscousFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rho_var_number)
  {
    // Derivatives wrt rho are much more lengthy than the velocity derivatives!
    
    // If rho==0.0, we can't do anything since we're about to divide by rho
    // all over the place!
    if (_rho[_qp] == 0.0)
      mooseError("Zero density detected at quadrature point!");

    // So we don't get confused with zero vs. 1-based indexing...
    const unsigned x_component=0, y_component=1, z_component=2;
    
    // ... 

    // Commonly-used values
    Real rho  = _rho[_qp];
    Real rho2 = rho*rho;
    Real rho3 = rho2*rho;
    Real phij = _phi[_j][_qp];
    
    // Kinematic viscosity, mu/rho
    Real mu = _dynamic_viscosity[_qp];
    Real nu = mu / rho;

    // Convenience variables
    RealVectorValue U(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);
    Real divU = _grad_rho_u[_qp](x_component) + _grad_rho_v[_qp](y_component) + _grad_rho_w[_qp](z_component);

    Real drhodx = _grad_rho[_qp](x_component);
    Real drhody = _grad_rho[_qp](y_component);
    Real drhodz = _grad_rho[_qp](z_component);

    Real dphijdx = _grad_phi[_j][_qp](x_component);
    Real dphijdy = _grad_phi[_j][_qp](y_component);
    Real dphijdz = _grad_phi[_j][_qp](z_component);

    Real U1 = _rho_u[_qp];
    Real U2 = _rho_v[_qp];
    Real U3 = _rho_w[_qp];

    Real dU1dx = _grad_rho_u[_qp](x_component);
    Real dU1dy = _grad_rho_u[_qp](y_component);
    Real dU1dz = _grad_rho_u[_qp](z_component);

    Real dU2dx = _grad_rho_v[_qp](x_component);
    Real dU2dy = _grad_rho_v[_qp](y_component);
    Real dU2dz = _grad_rho_v[_qp](z_component);

    Real dU3dx = _grad_rho_w[_qp](x_component);
    Real dU3dy = _grad_rho_w[_qp](y_component);
    Real dU3dz = _grad_rho_w[_qp](z_component);

    // ...

    // dU1/dx2 + dU2/dx1
    Real X12 = dU1dy + dU2dx;

    // dU1/dx3 + dU3/dx1
    Real X13 = dU1dz + dU3dx;

    // dU2/dx3 + dU3/dx2
    Real X23 = dU2dz + dU3dy;

    // ...

    // U1*dU0/dx2 + U2*dU0/dx1
    Real Y12 = U1*drhody + U2*drhodx;
    
    // U1*dU0/dx3 + U3*dU0/dx1
    Real Y13 = U1*drhodz + U3*drhodx;
    
    // U2*dU0/dx3 + U3*dU0/dx2
    Real Y23 = U2*drhodz + U3*drhody;

    // ...

    // U1*dphij/dx2 + U2*dphij/dx1
    Real Z12 = U1*dphijdy + U2*dphijdx;

    // U1*dphij/dx3 + U3*dphij/dx1
    Real Z13 = U1*dphijdz + U3*dphijdx;
    
    // U2*dphij/dx3 + U3*dphij/dx2
    Real Z23 = U2*dphijdz + U3*dphijdy;
    
    // ...
    
    // There are three unique "off-diagonal" terms, "off-diagonal" here refers to
    // dphii/dxk terms, where k != _component.  No matter what _component is, we 
    // will always use exactly two of these.
    Real off_diag_12 = nu * (-phij*X12/rho + 2.*phij*Y12/rho2 - Z12/rho);
    Real off_diag_13 = nu * (-phij*X13/rho + 2.*phij*Y13/rho2 - Z13/rho);
    Real off_diag_23 = nu * (-phij*X23/rho + 2.*phij*Y23/rho2 - Z23/rho);
    
    // "on-diagonal" term common to all components
    Real on_diag = (2./3.)*mu/rho2*( phij*divU - 2.*phij*(U*_grad_rho[_qp])/rho + (U*_grad_phi[_j][_qp]));
    
    // "on-diagonal terms specific to each component
    RealVectorValue on_diagv (// -dU1/dx1*phij + 2*phij/rho*U1*dU0/dx1 - U1*dphij/dx1
			      -dU1dx*phij 
			      + (2./rho)*U1*drhodx*phij
			      - U1*dphijdx, 
      
			      // -dU2/dx2*phij + 2*phij/rho*U2*dU0/dx2 - U2*dphij/dx2
			      -dU2dy*phij 
			      + (2./rho)*U2*drhody*phij
			      - U2*dphijdy, 

			      // -dU3/dx3*phij + 2*phij/rho*U3*dU0/dx3 - U3*dphij/dx3
			      -dU3dz*phij 
			      + (2./rho)*U3*drhodz*phij
			      - U3*dphijdz);
    
    // Scale component-specific on-diagonal terms
    on_diagv *= (2. * mu ) / rho2;
    
    // Fill in vector for dotting with test function gradient...
    RealVectorValue vec;
    vec(_component) = on_diag + on_diagv(_component);

    // Fill in off-diagonal terms
    switch(_component)
    {
    case x_component:
      vec(y_component) = off_diag_12;
      vec(z_component) = off_diag_13;
      break;
    case y_component:
      vec(x_component) = off_diag_12;
      vec(z_component) = off_diag_23;
      break;
    case z_component:
      vec(x_component) = off_diag_13;
      vec(y_component) = off_diag_23;
      break;
    default:
      mooseError("Invalid _component selected!");
    }

    // Finally, ready to return the value
    return (vec * _grad_test[_i][_qp]);
  }

  // Handle off-diagonal derivatives wrt momentums
  else if ((jvar == _rhou_var_number) || (jvar == _rhov_var_number) || (jvar == _rhow_var_number))
  {
    // Kinematic viscosity, mu/rho
    Real nu = _dynamic_viscosity[_qp] / _rho[_qp];

    // The alpha index (from my notes) is equal to the component
    unsigned alpha = _component;

    // Map jvar into beta = {0,1,2}, regardless of how Moose has numbered things.
    unsigned beta = 0;
    
    if (jvar == _rhov_var_number)
      beta = 1;
    else if (jvar == _rhow_var_number)
      beta = 2;

    // Make sure we are really doing off-diagonal couplings here
    mooseAssert(alpha!=beta, "Error, computing off-diagonal jacobian terms!");

    // Create a vector to be dotted with the test function for the return value
    RealVectorValue vec;
    
    // vec(alpha) = 2/3 * nu * (-dphi_j/dx_beta + (phi_j/rho)*drho/dx_beta)
    vec(alpha) = (2./3.) * nu * ( -_grad_phi[_j][_qp](beta) + (_phi[_j][_qp] / _rho[_qp])*_grad_rho[_qp](beta) );

    // vec(beta) = nu * (dphi_j/dx_alpha - (phi_j/rho)*drho/dx_alpha)
    vec(beta) = nu * ( _grad_phi[_j][_qp](alpha) - (_phi[_j][_qp] / _rho[_qp])*_grad_rho[_qp](alpha) );

    return (vec * _grad_test[_i][_qp]);
  }

  // If we got here, must be derivative wrt rho*E, which is zero for this kernel
  return 0.;
}
