#include "ACParsed.h"

template<>
InputParameters validParams<ACParsed>()
{
  InputParameters params = DerivativeKernelInterface<ACBulk>::validParams();
  params.addCoupledVar("args", "Vector of additional arguments to F");
  return params;
}

ACParsed::ACParsed(const std::string & name, InputParameters parameters) :
    DerivativeKernelInterface<ACBulk>(name, parameters),
    _dFdEta(getDerivative<Real>(_F_name, _var.name())),
    _d2FdEta2(getDerivative<Real>(_F_name, _var.name(), _var.name()))
{
}

Real
ACParsed::computeDFDOP(PFFunctionType type)
{
  switch (type)
  {
    case Residual:
      return _dFdEta[_qp];

    case Jacobian:
      return _d2FdEta2[_qp] * _phi[_j][_qp];
  }

  mooseError("Internal error");
}
