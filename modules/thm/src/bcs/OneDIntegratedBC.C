#include "OneDIntegratedBC.h"

template <>
InputParameters
validParams<OneDIntegratedBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<Real>("normal", "Component of outward normal along 1-D direction");
  return params;
}

OneDIntegratedBC::OneDIntegratedBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _normal(getParam<Real>("normal"))
{
}
