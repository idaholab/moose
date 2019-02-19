#include "OneDNodalBC.h"

template <>
InputParameters
validParams<OneDNodalBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addRequiredParam<Real>("normal", "Component of outward normal along 1-D direction");
  return params;
}

OneDNodalBC::OneDNodalBC(const InputParameters & parameters)
  : NodalBC(parameters), _normal(getParam<Real>("normal"))
{
}
