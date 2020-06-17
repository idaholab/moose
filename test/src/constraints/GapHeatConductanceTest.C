//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapHeatConductanceTest.h"

registerMooseObject("MooseTestApp", GapHeatConductanceTest);

InputParameters
GapHeatConductanceTest::validParams()
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
  return params;
}

GapHeatConductanceTest::GapHeatConductanceTest(const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _secondary_gap_conductance(getADMaterialProperty<Real>("secondary_gap_conductance")),
    _primary_gap_conductance(getNeighborADMaterialProperty<Real>("primary_gap_conductance"))
{
}

ADReal
GapHeatConductanceTest::computeQpResidual(Moose::MortarType type)
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
      if (_has_primary)
      {
        auto gap = (_phys_points_secondary[_qp] - _phys_points_primary[_qp]).norm();
        mooseAssert(MetaPhysicL::raw_value(gap) > TOLERANCE * TOLERANCE,
                    "Gap distance is too small in GapHeatConductanceTest");

        heat_transfer_coeff =
            (0.5 * (_secondary_gap_conductance[_qp] + _primary_gap_conductance[_qp])) / gap;
      }
      return _test[_i][_qp] *
             (_lambda[_qp] -
              heat_transfer_coeff * (_u_secondary[_qp] - (_has_primary ? _u_primary[_qp] : 0)));
    }

    default:
      return 0;
  }
}
