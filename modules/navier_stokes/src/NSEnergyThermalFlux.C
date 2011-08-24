#include "NSEnergyThermalFlux.h"

template<>
InputParameters validParams<NSEnergyThermalFlux>()
{
  InputParameters params = validParams<Kernel>();
  
  // Required coupled variables for residual terms
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





NSEnergyThermalFlux::NSEnergyThermalFlux(const std::string & name, InputParameters parameters)
    : Kernel(name, parameters),
      
      // Coupled variable values
      _rho(coupledValue("rho")),
      _rho_u(coupledValue("rhou")),
      _rho_v(coupledValue("rhov")),
      _rho_w( _dim == 3 ? coupledValue("rhow") : _zero),
      _rho_e(coupledValue("rhoe")),
   
      // Gradients
      _grad_temp(coupledGradient("temperature")),
      _grad_rho(coupledGradient("rho")),
      _grad_rho_u(coupledGradient("rhou")),
      _grad_rho_v(coupledGradient("rhov")),
      _grad_rho_w( _dim == 3 ? coupledGradient("rhow") : _grad_zero),
      _grad_rho_e(coupledGradient("rhoe")),
      
      // Variable numbers
      _rho_var_number( coupled("rho") ),
      _rhou_var_number( coupled("rhou") ),
      _rhov_var_number( coupled("rhov") ),
      _rhow_var_number( _dim == 3 ? coupled("rhow") : libMesh::invalid_uint),
      _rhoe_var_number( coupled("rhoe") ),

      // material properties and parameters
      _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),
      _cv(getParam<Real>("cv")),

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




Real
NSEnergyThermalFlux::computeQpResidual()
{
  // k * grad(T) * grad(phi)
  return _thermal_conductivity[_qp] * (_grad_temp[_qp] * _grad_test[_i][_qp]);
}





Real
NSEnergyThermalFlux::computeQpJacobian()
{
  // The "on-diagonal" Jacobian for the energy equation
  // corresponds to variable number 4.
  return this->compute_jacobian_value(/*var_number=*/4);
}




Real
NSEnergyThermalFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Map jvar into the numbering expected by this->compute_jacobain_value()
  unsigned var_number = 99; // invalid

  if (jvar==_rho_var_number)
    var_number = 0;
  else if (jvar==_rhou_var_number)
    var_number = 1;
  else if (jvar==_rhov_var_number)
    var_number = 2;
  else if (jvar==_rhow_var_number)
    var_number = 3;
  else
    mooseError("Invalid jvar!");

  return this->compute_jacobian_value(var_number);
}





Real NSEnergyThermalFlux::compute_jacobian_value(unsigned var_number)
{
  // The value to return
  Real result = 0.;

  // I used "ell" here as the loop counter since it matches the
  // "\ell" used in my LaTeX notes.
  for (unsigned int ell=0; ell<3; ++ell)
  {
    // Accumulate the first dot product term
    Real intermediate_result = _temp_derivs.get_grad(var_number) * _grad_phi[_j][_qp](ell);

    // Now accumulate the Hessian term
    Real hess_term = 0.;
    for (unsigned n=0; n<5; ++n)
    {
      // hess_term += get_hess(m,n) * gradU[n](ell); // ideally... but you can't have a vector<VariableGradient&> :-(
      hess_term += _temp_derivs.get_hess(var_number,n) * (*_gradU[n])[_qp](ell); // dereference pointer to get value
    }
    
    // Accumulate the second dot product term
    intermediate_result += hess_term * _phi[_j][_qp];

    // Hit intermediate_result with the test function, accumulate in the final value
    result += intermediate_result * _grad_test[_i][_qp](ell);
  }

  // Return result, don't forget to multiply by "k"!
  return _thermal_conductivity[_qp] * result;
}



