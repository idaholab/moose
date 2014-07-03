#include "SplitCHParsed.h"

template<>
InputParameters validParams<SplitCHParsed>()
{
  InputParameters params = ParsedFreeEnergyInterface<SplitCHCRes>::validParams();
  return params;
}

SplitCHParsed::SplitCHParsed(const std::string & name, InputParameters parameters) :
    ParsedFreeEnergyInterface<SplitCHCRes>(name, parameters)
{
}

Real
SplitCHParsed::computeDFDC(PFFunctionType type)
{
  updateFuncParams();

  switch (type)
  {
    case Residual:
      return firstDerivative(0);

    case Jacobian:
      return secondDerivative(0) * _phi[_j][_qp];
  }

  mooseError("Internal error");
}
