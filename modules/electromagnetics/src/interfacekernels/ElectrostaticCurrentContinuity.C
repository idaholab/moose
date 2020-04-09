#include "ElectrostaticCurrentContinuity.h"

registerMooseObject("ElkApp", ElectrostaticCurrentContinuity);

InputParameters
ElectrostaticCurrentContinuity::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addParam<MaterialPropertyName>(
      "master_conductivity", "electrical_conductivity", "Conductivity on the master block.");
  params.addParam<MaterialPropertyName>(
      "neighbor_conductivity", "electrical_conductivity", "Conductivity on the neighbor block.");
  params.addClassDescription(
      "Interface condition that describes the current continuity across a boundary formed between "
      "two dissimilar materials (resulting in a potential discontinuity). Conductivity on each "
      "side of the boundary is defined via the material peoperties system.");
  return params;
}

ElectrostaticCurrentContinuity::ElectrostaticCurrentContinuity(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _conductivity_master(getMaterialProperty<Real>("master_conductivity")),
    _conductivity_neighbor(getNeighborMaterialProperty<Real>("neighbor_conductivity"))
{
}

Real
ElectrostaticCurrentContinuity::computeQpResidual(Moose::DGResidualType type)
{
  switch (type)
  {
    case Moose::Element:
      return _conductivity_neighbor[_qp] * _grad_neighbor_value[_qp] * _normals[_qp] * _test[_i][_qp];

    case Moose::Neighbor:
      return _conductivity_master[_qp] * _grad_u[_qp] * _normals[_qp] * _test_neighbor[_i][_qp];
  }
}

Real
ElectrostaticCurrentContinuity::computeQpJacobian(Moose::DGJacobianType type)
{
  switch (type)
  {
    case Moose::ElementElement:
      return 0.0;

    case Moose::NeighborNeighbor:
      return 0.0;

    case Moose::NeighborElement:
      return _conductivity_master[_qp] * _grad_phi[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp];

    case Moose::ElementNeighbor:
      return _conductivity_neighbor[_qp] * _grad_phi_neighbor[_j][_qp] * _normals[_qp] * _test[_i][_qp];
  }
}
