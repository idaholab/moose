#include "FlowRegimeSimpleMaterial.h"

registerMooseObject("THMApp", FlowRegimeSimpleMaterial);

template <>
InputParameters
validParams<FlowRegimeSimpleMaterial>()
{
  InputParameters params = validParams<FlowRegimeBaseMaterial>();
  return params;
}

FlowRegimeSimpleMaterial::FlowRegimeSimpleMaterial(const InputParameters & parameters)
  : FlowRegimeBaseMaterial(parameters)
{
}

void
FlowRegimeSimpleMaterial::computeQpProperties()
{
  _kappa_liquid[_qp] = _alpha_liquid[_qp];
  _dkappa_liquid_dbeta[_qp] = _dalpha_liquid_dbeta[_qp];
}
