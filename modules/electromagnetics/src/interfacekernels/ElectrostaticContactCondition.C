#include "ElectrostaticContactCondition.h"

registerMooseObject("ElkApp", ElectrostaticContactCondition);

InputParameters
ElectrostaticContactCondition::validParams()
{
  InputParameters params = ADInterfaceKernel::validParams();
  params.addParam<MaterialPropertyName>(
      "master_conductivity", "electrical_conductivity", "Conductivity on the master block.");
  params.addParam<MaterialPropertyName>(
      "neighbor_conductivity", "electrical_conductivity", "Conductivity on the neighbor block.");
  params.addRequiredParam<Real>("electrical_contact_resistance",
                                "Electrical contact resistance coefficient.");
  params.addClassDescription(
      "Interface condition that describes the current continuity and contact resistance across a "
      "boundary formed between two dissimilar materials (resulting in a potential discontinuity), "
      "as described in Cincotti, et al (https://doi.org/10.1002/aic.11102). Conductivity on each "
      "side of the boundary is defined via the material peoperties system.");
  return params;
}

ElectrostaticContactCondition::ElectrostaticContactCondition(const InputParameters & parameters)
  : ADInterfaceKernel(parameters),
    _conductivity_master(getMaterialProperty<Real>("master_conductivity")),
    _conductivity_neighbor(getNeighborMaterialProperty<Real>("neighbor_conductivity")),
    _contact_resistance(getParam<Real>("electrical_contact_resistance"))
{
}

ADReal
ElectrostaticContactCondition::computeQpResidual(Moose::DGResidualType type)
{
  ADReal res = 0.0;

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
