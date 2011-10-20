#include "CHSplit2ChemPot.h"
// The couple, CHSplit2ChemPot and SplitCHBlackLZ, splits the CH equation by replacing chemical potential with 'w'.
template<>
InputParameters validParams<CHSplit2ChemPot>()
{
  InputParameters params = validParams<Kernel>();
  
  params.addRequiredCoupledVar("w","chem potential");
  params.addRequiredParam<std::string>("kappa_name","The kappa used with the kernel");

  return params;
}

CHSplit2ChemPot::CHSplit2ChemPot(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _w_var(coupled("w")),
   _w(coupledValue("w")),
   _grad_w(coupledGradient("w")),
   _kappa_name(getParam<std::string>("kappa_name")),
   _kappa(getMaterialProperty<Real>(_kappa_name))
{}

Real
CHSplit2ChemPot::computeDFDC(PFFunctionType type, Real c)
{
  switch (type)
  {
  case Residual:
    return 4.0*(c*c*c-c); // return Residual value
    
  case Jacobian: 
    return 4.0 * (3.0*c*c - 1.0)*_phi[_j][_qp]; //return Off-Diag Jacobian value
    
  }
  
  mooseError("Invalid type passed in");
}

Real
CHSplit2ChemPot::computeQpResidual()
{
  Real c = _u[_qp];
  RealGradient grad_c = _grad_u[_qp];
  
  Real f_prime_zero = computeDFDC(Residual,c);
  
  return (-_w[_qp]+f_prime_zero) * _test[_i][_qp] + _kappa[_qp]*grad_c*_grad_test[_i][_qp];
}

Real
CHSplit2ChemPot::computeQpJacobian()
{
  Real c = _u[_qp];
  Real df_prime_zero_dc = computeDFDC(Jacobian,c);

  return df_prime_zero_dc * _test[_i][_qp] + _kappa[_qp]*_grad_phi[_j][_qp]*_grad_test[_i][_qp];
}

Real
CHSplit2ChemPot::computeQpOffDiagJacobian(unsigned int jvar)
{

  if(jvar == _w_var)
  {
   return -_phi[_j][_qp] * _test[_i][_qp];
  }

  return 0.0;
}

