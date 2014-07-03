#include "CHParsed.h"

template<>
InputParameters validParams<CHParsed>()
{
  InputParameters params = ParsedFreeEnergyInterface<CHBulk>::validParams();
  return params;
}

CHParsed::CHParsed(const std::string & name, InputParameters parameters) :
    ParsedFreeEnergyInterface<CHBulk>(name, parameters)
{
}

RealGradient
CHParsed::computeGradDFDCons(PFFunctionType type)
{
  updateFuncParams();
  RealGradient res = 0.0;

  switch (type)
  {
    case Residual:
      for (unsigned int i = 0; i < _nvars; ++i)
        res += (*_grad_vars[i])[_qp] * secondDerivative(i);
      return res;

    case Jacobian:
      res = _grad_phi[_j][_qp] * secondDerivative(0);
      for (unsigned int i = 0; i < _nvars; ++i)
        res += _phi[_j][_qp] * (*_grad_vars[i])[_qp] * thirdDerivative(i);
      return res;
  }

  mooseError("Internal error");
}
