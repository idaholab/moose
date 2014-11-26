#include "SplitCHParsed.h"

template<>
InputParameters validParams<SplitCHParsed>()
{
  InputParameters params = DerivativeKernelInterface<SplitCHCRes>::validParams();
  return params;
}

SplitCHParsed::SplitCHParsed(const std::string & name, InputParameters parameters) :
    DerivativeKernelInterface<SplitCHCRes>(name, parameters),
    _dFdc(getMaterialPropertyDerivative<Real>(_F_name, _var.name())),
    _d2Fdc2(getMaterialPropertyDerivative<Real>(_F_name, _var.name(), _var.name()))
{
}

Real
SplitCHParsed::computeDFDC(PFFunctionType type)
{
  switch (type)
  {
    case Residual:
      return _dFdc[_qp];

    case Jacobian:
      return _d2Fdc2[_qp] * _phi[_j][_qp];
  }

  mooseError("Internal error");
}
