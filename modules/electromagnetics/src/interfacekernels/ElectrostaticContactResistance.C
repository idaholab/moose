#include "ElectrostaticContactResistance.h"

registerMooseObject("ElkApp", ElectrostaticContactResistance);

InputParameters
ElectrostaticContactResistance::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addParam<MaterialPropertyName>(
      "master_conductivity", "electrical_conductivity", "Conductivity on the master block.");
  params.addParam<MaterialPropertyName>(
      "neighbor_conductivity", "electrical_conductivity", "Conductivity on the neighbor block.");
  params.addRequiredParam<Real>("electrical_contact_resistance",
                                "Electrical contact resistance coefficient.");
  params.addClassDescription(
      "Interface condition that describes the current continuity and contact resistance across a "
      "boundary formed between two dissimilar materials (resulting in a potential discontinuity), "
      "as described in Cincotti, et al (DOI: 10.1002/aic). Conductivity on each side of the "
      "boundary is defined via the material peoperties system.");
  return params;
}

ElectrostaticContactResistance::ElectrostaticContactResistance(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _conductivity_master(getMaterialProperty<Real>("master_conductivity")),
    _conductivity_neighbor(getNeighborMaterialProperty<Real>("neighbor_conductivity")),
    _contact_resistance(getParam<Real>("electrical_contact_resistance"))
{
}

Real
ElectrostaticContactResistance::computeQpResidual(Moose::DGResidualType type)
{
  Real res = 0.0;

  switch (type)
  {
    case Moose::Element:
      res = 0.5 *
            (_conductivity_neighbor[_qp] * _grad_neighbor_value[_qp] * _normals[_qp] -
             _contact_resistance * (_neighbor_value[_qp] - _u[_qp])) *
            _test[_i][_qp];
      break;

    case Moose::Neighbor:
      res = _conductivity_master[_qp] * _grad_u[_qp] * _normals[_qp] * _test_neighbor[_i][_qp];
      break;
  }

  return res;
}

Real
ElectrostaticContactResistance::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0.0;

  switch (type)
  {
    case Moose::ElementElement:
      jac = 0.5 * _contact_resistance * _phi[_j][_qp] * _test[_i][_qp];
      break;

    case Moose::NeighborNeighbor:
      jac = 0.0;
      break;

    case Moose::NeighborElement:
      jac =
          _conductivity_master[_qp] * _grad_phi[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp];
      break;

    case Moose::ElementNeighbor:
      jac = 0.5 *
            (_conductivity_neighbor[_qp] * _grad_phi_neighbor[_j][_qp] * _normals[_qp] -
             _contact_resistance * _phi_neighbor[_j][_qp]) *
            _test[_i][_qp];
      break;
  }

  return jac;
}
