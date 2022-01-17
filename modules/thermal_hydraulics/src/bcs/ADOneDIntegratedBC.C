#include "ADOneDIntegratedBC.h"

InputParameters
ADOneDIntegratedBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<Real>("normal", "Component of outward normal along 1-D direction");
  return params;
}

ADOneDIntegratedBC::ADOneDIntegratedBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _normal(getParam<Real>("normal"))
{
}
