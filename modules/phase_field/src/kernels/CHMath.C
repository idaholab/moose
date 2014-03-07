#include "CHMath.h"

template<>
InputParameters validParams<CHMath>()
{
  InputParameters params = validParams<CHBulk>();

  return params;
}

CHMath::CHMath(const std::string & name, InputParameters parameters)
  :CHBulk(name, parameters)
{
}

RealGradient  //Use This an example of the the function should look like
CHMath::computeGradDFDCons(PFFunctionType type, Real c, RealGradient grad_c)
{
  switch (type)
  {
  case Residual:
    return 3*c*c*grad_c - grad_c; // return Residual value
    break;

  case Jacobian:
    return 6*c*_phi[_j][_qp]*grad_c + 3*c*c*_grad_phi[_j][_qp] - _grad_phi[_j][_qp]; //return Jacobian value
    break;

  default:
    mooseError("Invalid type passed in");
    break;
  }

  return 0.0;
}
