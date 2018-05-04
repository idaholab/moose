#include "PotentialToFieldAux.h"

registerMooseObject("ElkApp", PotentialToFieldAux);

template <>
InputParameters
validParams<PotentialToFieldAux>()
{
  InputParameters params = validParams<VariableGradientComponent>();
  params.addClassDescription("");
  MooseEnum sign("positive=1 negative=-1");
  params.addParam<MooseEnum>("sign", sign, "Sign of potential gradient.");
  return params;
}

PotentialToFieldAux::PotentialToFieldAux(const InputParameters & parameters)
  : VariableGradientComponent(parameters),

    _sign(getParam<MooseEnum>("sign"))
{
}

Real
PotentialToFieldAux::computeValue()
{
  return _sign * VariableGradientComponent::computeValue();
}
