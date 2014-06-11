#include "SplitCHMath.h"

// The couple, SplitCHMath and SplitCHWRes, splits the CH equation by replacing chemical potential with 'w'.
template<>
InputParameters validParams<SplitCHMath>()
{
  InputParameters params = validParams<SplitCHCRes>();

  return params;
}

SplitCHMath::SplitCHMath(const std::string & name, InputParameters parameters) :
    SplitCHCRes(name, parameters)
{
}

Real
SplitCHMath::computeDFDC(PFFunctionType type)
{
  switch (type)
  {
    case Residual:
      return _u[_qp]*_u[_qp]*_u[_qp] - _u[_qp]; // return Residual value

    case Jacobian:
      return (3.0*_u[_qp]*_u[_qp] - 1.0) * _phi[_j][_qp]; //return Jacobian value
  }

  mooseError("Invalid type passed in");
}
