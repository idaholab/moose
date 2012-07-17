#include "DGFunctionConvectionDirichletBC.h"
#include "Function.h"

#include "numeric_vector.h"

template<>
InputParameters validParams<DGFunctionConvectionDirichletBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addParam<Real>("value", 0.0, "The value the variable should have on the boundary");
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  params.addRequiredParam<Real>("x", "Component of the velocity in the x direction");
  params.addRequiredParam<Real>("y", "Component of the velocity in the y direction");
  params.addParam<Real>("z", 0.0, "Component of the velocity in the z direction");
  return params;
}

DGFunctionConvectionDirichletBC::DGFunctionConvectionDirichletBC(const std::string & name, InputParameters parameters) :
    IntegratedBC(name, parameters),
    _func(getFunction("function")),
    _x(getParam<Real>("x")),
    _y(getParam<Real>("y")),
    _z(getParam<Real>("z"))
{
  _velocity(0) = _x;
  _velocity(1) = _y;
  _velocity(2) = _z;
}

Real
DGFunctionConvectionDirichletBC::computeQpResidual()
{
  Real fn = _func.value(_t, _q_point[_qp]);
  Real r = 0;
  if (_velocity * _normals[_qp] >= 0)
  {
    r += (_velocity * _normals[_qp]) * (_u[_qp] - fn) * _test[_i][_qp];
  }
  else
  {
    r += (_velocity * _normals[_qp]) * (fn - _u[_qp]) * _test[_i][_qp];
  }

  return r;
}

Real
DGFunctionConvectionDirichletBC::computeQpJacobian()
{
  Real r = 0;
  if (_velocity * _normals[_qp] >= 0)
  {
    r += (_velocity * _normals[_qp]) * _test[_j][_qp] * _test[_i][_qp];
  }
  else
  {
    r -= (_velocity *_normals[_qp]) * _test[_j][_qp] * _test[_i][_qp];
  }

  return r;

}
