#include "CHSplit2ChemPot.h"
// The couple, CHSplit2ChemPot and SplitCHBlackLZ, splits the CH equation by replacing chemical potential with 'w'.
template<>
InputParameters validParams<CHSplit2ChemPot>()
{
  InputParameters params = validParams<Kernel>();
  
  params.addRequiredCoupledVar("c","concentration");

  return params;
}

CHSplit2ChemPot::CHSplit2ChemPot(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _c_var(coupled("c")),
   _c(coupledValue("c"))
{}

Real
CHSplit2ChemPot::computeDFDC(PFFunctionType type)
{
  switch (type)
  {
  case Residual:
    return 4.0*(_c[_qp]*_c[_qp]*_c[_qp]-_c[_qp]); // return Residual value
    
  case OffDiag: 
    return 4.0 * (3 * _c[_qp] * _c[_qp]  - 1.0)*_phi[_j][_qp]; //return Off-Diag Jacobian value
    
  }
  
  mooseError("Invalid type passed in");
}

Real
CHSplit2ChemPot::computeQpResidual()
{
  return (_u[_qp]-computeDFDC(Residual)) * _test[_i][_qp];
}

Real
CHSplit2ChemPot::computeQpJacobian()
{
  return _phi[_j][_qp]*_test[_i][_qp];
}

Real
CHSplit2ChemPot::computeQpOffDiagJacobian(unsigned int jvar)
{

  if(jvar == _c_var)
  {  

   return -computeDFDC(OffDiag) * _test[_i][_qp]; 


  }

  return 0.0;
}

