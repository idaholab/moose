#include "CHBulkPFCTrad.h"
#include "MathUtils.h"
using namespace MathUtils;

template<>
InputParameters validParams<CHBulkPFCTrad>()
{
  InputParameters params = validParams<CHBulk>();

  return params;
}

CHBulkPFCTrad::CHBulkPFCTrad(const std::string & name, InputParameters parameters)
  :CHBulk(name, parameters),
   _C0(getMaterialProperty<Real>("C0")),
   _a(getMaterialProperty<Real>("a")),
   _b(getMaterialProperty<Real>("b"))
{
}

RealGradient
CHBulkPFCTrad::computeGradDFDCons(PFFunctionType type, Real c, RealGradient grad_c)
{
  
  Real d2fdc2 = 1.0 - _C0[_qp] - _a[_qp]*c + _b[_qp]*c*c;
    
  switch (type)
  {
    
  case Residual:
    return d2fdc2*grad_c;
    break;
    
  case Jacobian:
    Real d3fdc3 = - _a[_qp] + 2.0*_b[_qp]*c;
    return d2fdc2*_grad_phi[_j][_qp] + d3fdc3*grad_c*_phi[_j][_qp];
    break;
    
  }
  
  mooseError("Invalid type passed in");
}
