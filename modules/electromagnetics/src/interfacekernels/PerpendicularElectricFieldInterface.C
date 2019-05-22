#include "PerpendicularElectricFieldInterface.h"

registerMooseObject("ElkApp", PerpendicularElectricFieldInterface);

template <>
InputParameters
validParams<PerpendicularElectricFieldInterface>()
{
  InputParameters params = validParams<VectorInterfaceKernel>();
  params.addClassDescription("VectorInterfaceKernel that implements the condtion $\\epsilon_1 \\vec{E}_{1}^{\\perp} - \\epsilon_2 \\vec{E}_{2}^{\\perp} = \\sigma_f$");
  params.addParam<Real>("free_charge", 0.0, "Free charge on the interface (default = 0).");
  params.addParam<Real>("master_epsilon", 1.0, "Permittivity on the master side of the interface (default = 1.0).");
  params.addParam<Real>("neighbor_epsilon", 1.0, "Permittivity on the neighbor side of the interface (default = 1.0).");
  return params;
}

PerpendicularElectricFieldInterface::PerpendicularElectricFieldInterface(const InputParameters & parameters)
  : VectorInterfaceKernel(parameters),

  _free_charge(getParam<Real>("free_charge")),
  _master_eps(getParam<Real>("master_epsilon")),
  _neighbor_eps(getParam<Real>("neighbor_epsilon"))
{
}

Real
PerpendicularElectricFieldInterface::computeQpResidual(Moose::DGResidualType type)
{
  _u_perp = (_u[_qp] * _normals[_qp]) * _normals[_qp];
  _neighbor_perp = (_neighbor_value[_qp] * _normals[_qp]) * _normals[_qp];

  Real res = 0;

  switch (type)
  {
    case Moose::Element:
      res = _test[_i][_qp] * (_master_eps * _u_perp - _neighbor_eps * _neighbor_perp);
      break;

    case Moose::Neighbor:
      res =  _test_neighbor[_i][_qp] * -(_master_eps * _u_perp - _neighbor_eps * _neighbor_perp);
      break;
  }

  return res;
}

Real
PerpendicularElectricFieldInterface::computeQpJacobian(Moose::DGJacobianType type)
{
  _phi_u_perp = (_phi[_j][_qp] * _normals[_qp]) * _normals[_qp];
  _phi_neighbor_perp = (_phi_neighbor[_j][_qp] * _normals[_qp]) * _normals[_qp];

  Real jac = 0;

  switch (type)
  {
    case Moose::ElementElement:
      jac = _test[_i][_qp] * _master_eps * _phi_u_perp;
      break;

    case Moose::NeighborNeighbor:
      jac = _test_neighbor[_i][_qp] * _neighbor_eps * _phi_neighbor_perp;
      break;

    case Moose::NeighborElement:
      jac = _test_neighbor[_i][_qp] * -_master_eps * _phi_u_perp;
      break;

    case Moose::ElementNeighbor:
      jac = _test[_i][_qp] * -_neighbor_eps * _phi_neighbor_perp;
      break;
  }

  return jac;
}
