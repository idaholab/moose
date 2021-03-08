//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapConductanceConstraint.h"

registerMooseObject("HeatConductionApp", GapConductanceConstraint);

InputParameters
GapConductanceConstraint::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addClassDescription(
      "Computes the residual and Jacobian contributions for the 'Lagrange Multiplier' "
      "implementation of the thermal contact problem. For more information, see the "
      "detailed description here: http://tinyurl.com/gmmhbe9");
  params.addRequiredParam<Real>("k", "Gap conductance");
  params.addParam<Real>("min_gap",
                        1e-6,
                        "The minimum gap distance allowed. This helps with preventing the heat "
                        "flux from going to infinity as the gap approaches zero.");
  return params;
}

GapConductanceConstraint::GapConductanceConstraint(const InputParameters & parameters)
  : ADMortarConstraint(parameters), _k(getParam<Real>("k")), _min_gap(getParam<Real>("min_gap"))
{
}

ADReal
GapConductanceConstraint::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Primary:
      return _lambda[_qp] * _test_primary[_i][_qp];
    case Moose::MortarType::Secondary:
      return -_lambda[_qp] * _test_secondary[_i][_qp];
    case Moose::MortarType::Lower:
    {
      auto l = std::max((_phys_points_primary[_qp] - _phys_points_secondary[_qp]) * _normals[_qp],
                        _min_gap);
      return (_lambda[_qp] - _k * (_u_primary[_qp] - _u_secondary[_qp]) / l) * _test[_i][_qp];
    }
    default:
      return 0;
  }
}
