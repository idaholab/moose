#include "DGAdvection.h"

template<>
InputParameters validParams<DGAdvection>()
{
  InputParameters params = validParams<DGKernel>();
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity vector");
  return params;
}

DGAdvection::DGAdvection(const InputParameters & parameters) :
    DGKernel(parameters),
    _velocity(getParam<RealVectorValue>("velocity"))
{}

Real
DGAdvection::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;

  switch (type)
  {
    case Moose::Element:
      if ( (_velocity * _normals[_qp]) >= 0.0)
        r += (_velocity * _normals[_qp]) * _u[_qp] * _test[_i][_qp];
      else
        r += (_velocity * _normals[_qp]) * _u_neighbor[_qp] * _test[_i][_qp];
      break;

    case Moose::Neighbor:
      if ( (_velocity * _normals[_qp]) >= 0.0)
        r += -(_velocity * _normals[_qp]) * _u[_qp] * _test_neighbor[_i][_qp];
      else
        r += -(_velocity * _normals[_qp]) * _u_neighbor[_qp] * _test_neighbor[_i][_qp];
      break;
  }
  return r;
}

Real
DGAdvection::computeQpJacobian(Moose::DGJacobianType type)
{
  Real r = 0;

  switch (type)
  {

    case Moose::ElementElement:
      if ( (_velocity * _normals[_qp]) >= 0.0)
        r += (_velocity * _normals[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
      else
        r += 0.0;
      break;

    case Moose::ElementNeighbor:
      if ( (_velocity * _normals[_qp]) >= 0.0)
        r += 0.0;
      else
        r += (_velocity * _normals[_qp]) * _phi_neighbor[_j][_qp] * _test[_i][_qp];
      break;

    case Moose::NeighborElement:
      if ( (_velocity * _normals[_qp]) >= 0.0)
        r += -(_velocity * _normals[_qp]) * _phi[_j][_qp] * _test_neighbor[_i][_qp];
      else
        r += 0.0;
      break;

    case Moose::NeighborNeighbor:
      if ( (_velocity * _normals[_qp]) >= 0.0)
        r += 0.0;
      else
        r += -(_velocity * _normals[_qp]) * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
      break;
  }
  return r;
}
