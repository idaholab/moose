#include "HeatConvectionInterface.h"

#include "metaphysicl/raw_type.h"

using MetaPhysicL::raw_value;

registerMooseObject("HeatConductionApp", HeatConvectionInterface);

InputParameters
HeatConvectionInterface::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("htc", "heat transfer coefficient");
  return params;
}

HeatConvectionInterface::HeatConvectionInterface(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _h(getADMaterialPropertyByName<Real>(this->template getParam<MaterialPropertyName>("htc")))
{
}

Real
HeatConvectionInterface::computeQpResidual(Moose::DGResidualType type)
{
  switch (type)
  {
    case Moose::Element:
      return raw_value(_h[_qp]) * (_u[_qp] - _neighbor_value[_qp]) * _test[_i][_qp];

    case Moose::Neighbor:
      return raw_value(_h[_qp]) * (_neighbor_value[_qp] - _u[_qp]) * _test_neighbor[_i][_qp];

    default:
      return 0.0;
  }
}

Real
HeatConvectionInterface::computeQpJacobian(Moose::DGJacobianType type)
{
  switch (type)
  {
    case Moose::ElementElement:
      return raw_value(_h[_qp]) * _phi[_j][_qp] * _test[_i][_qp];

    case Moose::NeighborNeighbor:
      return raw_value(_h[_qp]) * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];

    case Moose::NeighborElement:
      return raw_value(_h[_qp]) * -_phi[_j][_qp] * _test_neighbor[_i][_qp];

    case Moose::ElementNeighbor:
      return raw_value(_h[_qp]) * -_phi_neighbor[_j][_qp] * _test[_i][_qp];

    default:
      return 0.0;
  }
}
