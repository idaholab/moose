#include "EnergyThermalFlux.h"

template<>
InputParameters validParams<EnergyThermalFlux>()
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





EnergyThermalFlux::EnergyThermalFlux(const std::string & name, InputParameters parameters)
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
      _cv(getParam<Real>("cv"))
{
  // Zero out the gradient and Hessian entries so we are never dealing with uninitialized memory
  for (unsigned i=0; i<5; ++i)
  {
    _dTdU[i] = 0.;
    for (unsigned j=0; j<5; ++j)
      _hessian[i][j] = 0.;
  }

  // Store pointers to all variable gradients in a single vector.
  _gradU.resize(5);
  _gradU[0] = &_grad_rho  ;
  _gradU[1] = &_grad_rho_u;
  _gradU[2] = &_grad_rho_v;
  _gradU[3] = &_grad_rho_w;
  _gradU[4] = &_grad_rho_e;
}




Real
EnergyThermalFlux::computeQpResidual()
{
  // k * grad(T) * grad(phi)
  return _thermal_conductivity[_qp] * (_grad_temp[_qp] * _grad_test[_i][_qp]);
}





Real
EnergyThermalFlux::computeQpJacobian()
{
  // The "on-diagonal" Jacobian for the energy equation
  // corresponds to variable number 4.
  return this->compute_jacobian_value(/*var_number=*/4);
}




Real
EnergyThermalFlux::computeQpOffDiagJacobian(unsigned int jvar)
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





Real EnergyThermalFlux::compute_jacobian_value(unsigned var_number)
{
  // Recompute gradient and Hessian data at this quadrature point.
  this->recalculate_gradient_and_hessian();

  // The value to return
  Real result = 0.;

  // I used "ell" here as the loop counter since it matches the
  // "\ell" used in my LaTeX notes.
  for (unsigned int ell=0; ell<3; ++ell)
  {
    // Accumulate the first dot product term
    Real intermediate_result = _dTdU[var_number] * _grad_phi[_j][_qp](ell);

    // Now accumulate the Hessian term
    Real hess_term = 0.;
    for (unsigned n=0; n<5; ++n)
    {
      // hess_term += get_hess(m,n) * gradU[n](ell); // ideally... but you can't have a vector<VariableGradient&> :-(
      hess_term += get_hess(var_number,n) * (*_gradU[n])[_qp](ell); // dereference pointer to get value
    }
    
    // Accumulate the second dot product term
    intermediate_result += hess_term * _phi[_j][_qp];

    // Hit intermediate_result with the test function, accumulate in the final value
    result += intermediate_result * _grad_test[_i][_qp](ell);
  }

  // Return result, don't forget to multiply by "k"!
  return _thermal_conductivity[_qp] * result;
}



void EnergyThermalFlux::recalculate_gradient_and_hessian()
{
  // Convenience variables
  Real U0 = _rho[_qp];
  Real U1 = _rho_u[_qp];
  Real U2 = _rho_v[_qp];
  Real U3 = _rho_w[_qp];
  Real U4 = _rho_e[_qp];
  
  Real mom2 = U1*U1 + U2*U2 + U3*U3;
  Real U02 = U0*U0;  // rho^2
  Real U03 = U02*U0; // rho^3
  Real U04 = U03*U0; // rho^4
  
  // This temporary will be used several times in the Hessian
  const Real temp = -1./U02/_cv;

  // ...



  // Gradient entries...

  _dTdU[0] =  (U4 - (mom2/U0))*temp;
  _dTdU[1] =  U1*temp;               
  _dTdU[2] =  U2*temp;               
  _dTdU[3] =  U3*temp;               
  _dTdU[4] = -U0*temp;
    
  

  // Hessian entries...

  // Note: only the lower triangle is filled in, the matrix is symmetric.

  // Row 0
  _hessian[0][0] = 2.*U4/U03/_cv - 3.*mom2/U04/_cv;

  // Row 1
  _hessian[1][0] = 2.*U1/U03/_cv;
  _hessian[1][1] = temp;

  // Row 2
  _hessian[2][0] = 2.*U2/U03/_cv;
  _hessian[2][2] = temp;

  // Row 3
  _hessian[3][0] = 2.*U3/U03/_cv;
  _hessian[3][3] = temp;
  
  // Row 4
  _hessian[4][0] = temp;
}





Real EnergyThermalFlux::get_hess(unsigned i, unsigned j)
{
  // If in lower triangle, OK
  if (i<=j)
    return _hessian[i][j];

  // Otherwise, convert passed in upper-triangular entry to
  // lower-triangular.
  return _hessian[j][i];
}




