#include "PotentialToFieldAux.h"

registerMooseObject("ElkApp", PotentialToFieldAux);

InputParameters
PotentialToFieldAux::validParams()
{
  InputParameters params = VariableGradientComponent::validParams();
  params.addClassDescription("An AuxKernel that calculates the electrostatic electric field given "
                             "the electrostatic potential.");
  MooseEnum sign("positive=1 negative=-1", "negative");
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
