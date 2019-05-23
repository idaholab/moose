#include "ParallelElectricFieldInterface.h"

registerMooseObject("ElkApp", ParallelElectricFieldInterface);

template <>
InputParameters
validParams<ParallelElectricFieldInterface>()
{
  InputParameters params = validParams<VectorInterfaceKernel>();
  params.addClassDescription("VectorInterfaceKernel that implements the condtion $\\vec{E}_{1}^{\\parallel} - \\vec{E}_{2}^{\\parallel} = 0$");
  return params;
}

ParallelElectricFieldInterface::ParallelElectricFieldInterface(const InputParameters & parameters)
  : VectorInterfaceKernel(parameters)
{
}

Real
ParallelElectricFieldInterface::computeQpResidual(Moose::DGResidualType type)
{
  _u_parallel = -_normals[_qp].cross(_normals[_qp].cross(_u[_qp]));
  _neighbor_parallel = -_normals[_qp].cross(_normals[_qp].cross(_neighbor_value[_qp]));

  Real res = 0;

  switch (type)
  {
    case Moose::Element:
      res = _test[_i][_qp] * (_u_parallel - _neighbor_parallel);
      break;

    case Moose::Neighbor:
      res =  _test_neighbor[_i][_qp] * -(_u_parallel - _neighbor_parallel);
      break;
  }

  return res;
}

Real
ParallelElectricFieldInterface::computeQpJacobian(Moose::DGJacobianType type)
{
  _phi_u_parallel = -_normals[_qp].cross(_normals[_qp].cross(_phi[_j][_qp]));
  _phi_neighbor_parallel = -_normals[_qp].cross(_normals[_qp].cross(_phi_neighbor[_j][_qp]));

  Real jac = 0;

  switch (type)
  {
    case Moose::ElementElement:
      jac = _test[_i][_qp] * _phi_u_parallel;
      break;

    case Moose::NeighborNeighbor:
      jac = _test_neighbor[_i][_qp] * _phi_neighbor_parallel;
      break;

    case Moose::NeighborElement:
      jac = _test_neighbor[_i][_qp] * -_phi_u_parallel;
      break;

    case Moose::ElementNeighbor:
      jac = _test[_i][_qp] * -_phi_neighbor_parallel;
      break;
  }

  return jac;
}
