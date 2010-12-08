#include "DGConvection.h"

template<>
InputParameters validParams<DGConvection>()
{
  InputParameters params = validParams<DGKernel>();
  params.addRequiredParam<Real>("x", "Component of velocity in the x direction");
  params.addRequiredParam<Real>("y", "Component of velocity in the y direction");
  params.addParam<Real>("z", 0.0, "Component of velocity in the z direction");
  return params;
}

DGConvection::DGConvection(const std::string & name, InputParameters parameters)
  :DGKernel(name, parameters),
   _x(getParam<Real>("x")),
   _y(getParam<Real>("y")),
   _z(getParam<Real>("z"))
{
  _velocity(0)=_x;
  _velocity(1)=_y;
  _velocity(2)=_z;
}

Real DGConvection::computeQpResidual(DGResidualType type)
{
  Real r = 0;
  Real _u_up;
  if (_velocity * _normals[_qp] >= 0)
  {
    _u_up = _u[_qp];
  }
  else
  {
    _u_up = _u_neighbor[_qp];
  }

  switch (type)
  {
  case Element:
    r += (_velocity * _normals[_qp]) *_u_up * _test[_i][_qp];
    break;

  case Neighbor:
    r -= (_velocity * _normals[_qp]) *_u_up * _test_neighbor[_i][_qp];
    break;
  }

  return r;
}

Real DGConvection::computeQpJacobian(DGJacobianType type)
{
  Real r = 0;

  if(_velocity * _normals[_qp] >= 0)
  {
    switch (type)
    {
    case ElementElement:
      r += (_velocity * _normals[_qp]) *_test[_j][_qp] * _test[_i][_qp];
      break;

    case ElementNeighbor:
      break;

    case NeighborElement:
      r -= (_velocity * _normals[_qp]) *_test[_j][_qp] * _test_neighbor[_i][_qp];
      break;

    case NeighborNeighbor:
      break;
    }
  }
  else
  {
    switch (type)
    {
    case ElementElement:
      break;

    case ElementNeighbor:
      r += (_velocity * _normals[_qp]) *_test_neighbor[_j][_qp] * _test[_i][_qp];
      break;

    case NeighborElement:
      break;

    case NeighborNeighbor:
      r -= (_velocity * _normals[_qp]) *_test_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
      break;
    }
  }

  return r;

}
