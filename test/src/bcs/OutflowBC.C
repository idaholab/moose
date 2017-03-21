#include "OutflowBC.h"
#include "Function.h"

template <>
InputParameters
validParams<OutflowBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<RealVectorValue>("velocity", "The velocity vector");
  return params;
}

OutflowBC::OutflowBC(const InputParameters & parameters)
  : IntegratedBC(parameters),

    _velocity(getParam<RealVectorValue>("velocity"))
{
}

Real
OutflowBC::computeQpResidual()
{
  return _test[_i][_qp] * _u[_qp] * _velocity * _normals[_qp];
}

Real
OutflowBC::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp] * _velocity * _normals[_qp];
}
