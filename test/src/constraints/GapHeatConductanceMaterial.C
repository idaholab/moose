//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapHeatConductanceMaterial.h"

registerMooseObject("MooseTestApp", GapHeatConductanceMaterial);

InputParameters
GapHeatConductanceMaterial::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addParam<Real>("gap_conductance_constant",
                        0.03,
                        "The numerator in calculation of the heat transfer coefficient");
  params.addParam<MaterialPropertyName>(
      "secondary_gap_conductance",
      "gap_conductance",
      "The material property name providing the gap conductance on the secondary side");
  params.addParam<MaterialPropertyName>(
      "primary_gap_conductance",
      "gap_conductance",
      "The material property name providing the gap conductance on the primary side");
  params.addParam<MaterialPropertyName>(
      "material_property",
      "layer_modifier",
      "The boundary-restricted material property name providing a flux modifier");

  return params;
}

GapHeatConductanceMaterial::GapHeatConductanceMaterial(const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _secondary_gap_conductance(getADMaterialProperty<Real>("secondary_gap_conductance")),
    _primary_gap_conductance(getNeighborADMaterialProperty<Real>("primary_gap_conductance")),
    _layer_modifier(getADMaterialProperty<Real>("layer_modifier"))
{
}

ADReal
GapHeatConductanceMaterial::computeQpResidual(Moose::MortarType type)
{
  switch (type)
  {
    case Moose::MortarType::Secondary:
      return _lambda[_qp] * _test_secondary[_i][_qp];

    case Moose::MortarType::Primary:
      return -_lambda[_qp] * _test_primary[_i][_qp];

    case Moose::MortarType::Lower:
    {
      ADReal heat_transfer_coeff(0);
      auto gap = (_phys_points_secondary[_qp] - _phys_points_primary[_qp]).norm();
      mooseAssert(MetaPhysicL::raw_value(gap) > TOLERANCE * TOLERANCE,
                  "Gap distance is too small in GapHeatConductanceMaterial");

      heat_transfer_coeff =
          (0.5 * (_secondary_gap_conductance[_qp] + _primary_gap_conductance[_qp])) / gap;

      // Modify heat transfer coefficient with boundary-restricted material property
      heat_transfer_coeff *= 0.5 * _layer_modifier[_qp];

      return _test[_i][_qp] *
             (_lambda[_qp] - heat_transfer_coeff * (_u_secondary[_qp] - _u_primary[_qp]));
    }

    default:
      return 0;
  }
}
