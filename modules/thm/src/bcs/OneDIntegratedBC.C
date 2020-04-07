#include "OneDIntegratedBC.h"

InputParameters
OneDIntegratedBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<Real>("normal", "Component of outward normal along 1-D direction");
  return params;
}

OneDIntegratedBC::OneDIntegratedBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _normal(getParam<Real>("normal"))
{
}
