#include "NSEnergyBC.h"

template<>
InputParameters validParams<NSEnergyBC>()
{
  InputParameters params = validParams<NSIntegratedBC>();

  // Required coupled variables for residual terms
  params.addRequiredCoupledVar("temperature", "");

  // Parameters with default values
  params.addRequiredParam<Real>("cv", "Specific heat at constant volume");
  
  return params;
}



NSEnergyBC::NSEnergyBC(const std::string & name, InputParameters parameters)
    : NSIntegratedBC(name, parameters),
      
      // Coupled gradients
      _grad_temperature(coupledGradient("temperature")),

      // Material properties
      _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),
      
      // Parameters
      _cv(getParam<Real>("cv")),
      
      // Viscous stress tensor derivative computing object
      _vst_derivs(*this),

      // Temperature derivative computing object
      _temp_derivs(*this)
{
  // Store pointers to all variable gradients in a single vector.
  _gradU.resize(5);
  _gradU[0] = &_grad_rho  ;
  _gradU[1] = &_grad_rho_u;
  _gradU[2] = &_grad_rho_v;
  _gradU[3] = &_grad_rho_w;
  _gradU[4] = &_grad_rho_e;
}




Real NSEnergyBC::computeQpResidual()
{
  // n . (rho*H*u - k*grad(T) - tau*u) v
  
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // rho*H*u = (rho*E + p)*u
  RealVectorValue conv_vec = (_rho_e[_qp] + _specified_pressure) * vel;

  // k*grad(T)
  RealVectorValue thermal_vec = _thermal_conductivity[_qp] * _grad_temperature[_qp];

  // tau*u
  RealVectorValue visc_vec = _viscous_stress_tensor[_qp] * vel;

  // Add everything up, dot with normal, hit with test function.
  return ((conv_vec - thermal_vec - visc_vec) * _normals[_qp]) * _test[_i][_qp];
}




Real NSEnergyBC::computeQpJacobian()
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Derivative of convective term (above) wrt U_4
  Real conv_term = _phi[_j][_qp] * (vel * _normals[_qp]);

  // See notes for this term, involves temperature Hessian 
  Real thermal_term = 0.;
  
  for (unsigned ell=0; ell<LIBMESH_DIM; ++ell)
  {
    Real intermediate_result = 0.;

    // The temperature Hessian contribution
    for (unsigned n=0; n<5; ++n)
      intermediate_result += _temp_derivs.get_hess(/*m=*/4, n) * (*_gradU[n])[_qp](ell);

    // Hit Hessian contribution with test function
    intermediate_result *= _phi[_j][_qp];
    
    // Add in the temperature gradient contribution
    intermediate_result += _temp_derivs.get_grad(/*rhoe=*/4) * _grad_phi[_j][_qp](ell);

    // Hit the result with the normal component, accumulate in thermal_term
    thermal_term += intermediate_result * _normals[_qp](ell);
  }

  // Hit thermal_term with thermal conductivity
  thermal_term *= _thermal_conductivity[_qp];

  return (conv_term - thermal_term) * _test[_i][_qp];
}




Real NSEnergyBC::computeQpOffDiagJacobian(unsigned jvar)
{
  // Note: This function requires both _vst_derivs *and* _temp_derivs

  // Convenience variables
  RealTensorValue& tau = _viscous_stress_tensor[_qp];
  
  Real rho  = _rho[_qp];
  Real phij = _phi[_j][_qp];
  RealVectorValue U(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);

  // H_bar = E + p_bar/rho, the enthalpy evaluated at the specified pressure
  Real H_bar = (_rho_e[_qp] + _specified_pressure) / _rho[_qp];

  // Map jvar into the variable m for our problem, regardless of
  // how Moose has numbered things. 
  unsigned m = 99;
 
  if (jvar == _rho_var_number)
    m = 0;
  else if (jvar == _rhou_var_number)
    m = 1;
  else if (jvar == _rhov_var_number)
    m = 2;
  else if (jvar == _rhow_var_number)
    m = 3;
  else if (jvar == _rhoe_var_number)
    m = 4;
  else
    mooseError("Invalid jvar!");

  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);


  // 
  // 1.) Convective term derivatives
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

  //
  // 2.) Thermal term derivatives
  //

  // See notes for this term, involves temperature Hessian 
  Real thermal_term = 0.;
  
  for (unsigned ell=0; ell<LIBMESH_DIM; ++ell)
  {
    Real intermediate_result = 0.;

    // The temperature Hessian contribution
    for (unsigned n=0; n<5; ++n)
      intermediate_result += _temp_derivs.get_hess(m, n) * (*_gradU[n])[_qp](ell);

    // Hit Hessian contribution with test function
    intermediate_result *= _phi[_j][_qp];
    
    // Add in the temperature gradient contribution
    intermediate_result += _temp_derivs.get_grad(m) * _grad_phi[_j][_qp](ell);

    // Hit the result with the normal component, accumulate in thermal_term
    thermal_term += intermediate_result * _normals[_qp](ell);
  }

  // Hit thermal_term with thermal conductivity
  thermal_term *= _thermal_conductivity[_qp];

  // 
  // 3.) Viscous term derivatives
  //

  // Compute viscous term derivatives
  Real visc_term = 0.;

  switch ( m )
  {
    
  case 0: // density
  {
    // Loop over k and ell as in the notes...
    for (unsigned k=0; k<LIBMESH_DIM; ++k)
    {
      Real intermediate_value = 0.;

      for (unsigned ell=0; ell<LIBMESH_DIM; ++ell)
	intermediate_value += ( (U(ell)/rho)*(-tau(k,ell)*phij/rho + _vst_derivs.dtau(k,ell,m)) );
      
      // Hit accumulated value with normal component k.  We will multiply by test function at 
      // the end of this routine...
      visc_term += intermediate_value * _normals[_qp](k);
    } // end for k

    break;
  } // end case 0

  case 1:
  case 2:
  case 3: // momentums
  {
    // Map m -> 0,1,2 as usual...
    unsigned m_local = m-1;
    
    // Loop over k and ell as in the notes...
    for (unsigned k=0; k<LIBMESH_DIM; ++k)
    {
      Real intermediate_value = tau(k,m_local)*phij/rho;
      
      for (unsigned ell=0; ell<LIBMESH_DIM; ++ell)
	intermediate_value += _vst_derivs.dtau(k,ell,m) * U(ell)/rho; // Note: pass 'm' to dtau, it will convert it internally

      // Hit accumulated value with normal component k.
      visc_term += intermediate_value * _normals[_qp](k);
    } // end for k
    
    break;
  } // end case 1,2,3

  case 4: // energy
    mooseError("Shouldn't get here, this is the on-diagonal entry!");

  default:
    mooseError("Invalid m value.");
  }

  // Finally, sum up the different contributions (with appropriate
  // sign) multiply by the test function, and return.
  return (conv_term - thermal_term - visc_term) * _test[_i][_qp];
}
