//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EqualValueConstraint.h"
#include "SubProblem.h"
#include "FEProblem.h"
#include "Assembly.h"

registerMooseObject("MooseApp", EqualValueConstraint);

InputParameters
EqualValueConstraint::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addClassDescription(
      "EqualValueConstraint enforces solution continuity between secondary and "
      "primary sides of a mortar interface using lagrange multipliers");
  params.addRangeCheckedParam<Real>(
      "delta", 0, "0<=delta<=1", "The coefficient for stabilizing terms");
  params.addParam<MaterialPropertyName>(
      "diff_secondary", 1, "The diffusivity on the secondary side");
  params.addParam<MaterialPropertyName>("diff_primary", 1, "The diffusivity on the primary side");
  return params;
}

EqualValueConstraint::EqualValueConstraint(const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _lower_secondary_volume(_assembly.lowerDElemVolume()),
    _lower_primary_volume(_assembly.neighborLowerDElemVolume()),
    _delta(getParam<Real>("delta")),
    _diff_secondary(getADMaterialProperty<Real>("diff_secondary")),
    _diff_primary(getADMaterialProperty<Real>("diff_primary")),
    _stabilize(_delta > TOLERANCE * TOLERANCE)
{
}

ADReal
EqualValueConstraint::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Secondary:
    {
      // The sign choice here makes it so that for the true solution: lambda = normals_secondary *
      // diff_secondary * grad_u_secondary
      auto residual = -_lambda[_qp] * _test_secondary[_i][_qp];

      if (_stabilize)
        residual += _delta * _lower_secondary_volume *
                    (_diff_secondary[_qp] * _grad_test_secondary[_i][_qp] * _normals[_qp]) *
                    (_lambda[_qp] - _diff_secondary[_qp] * _grad_u_secondary[_qp] * _normals[_qp]);

      return residual;
    }

    case Moose::MortarType::Primary:
    {
      // The sign choice here makes it so that for the true solution: lambda = -normals_primary *
      // diff_primary * grad_u_primary
      auto residual = _lambda[_qp] * _test_primary[_i][_qp];

      if (_stabilize)
        residual +=
            _delta * _lower_primary_volume *
            (_diff_primary[_qp] * _grad_test_primary[_i][_qp] * _normals_primary[_qp]) *
            (-_lambda[_qp] - _diff_primary[_qp] * _grad_u_primary[_qp] * _normals_primary[_qp]);

      return residual;
    }

    case Moose::MortarType::Lower:
    {
      auto residual = (_u_primary[_qp] - _u_secondary[_qp]) * _test[_i][_qp];

      if (_stabilize)
      {
        // secondary
        residual -= _delta * _lower_secondary_volume * _test[_i][_qp] *
                    (_lambda[_qp] - _diff_secondary[_qp] * _grad_u_secondary[_qp] * _normals[_qp]);

        // primary
        residual -=
            _delta * _lower_primary_volume * _test[_i][_qp] *
            (_lambda[_qp] + _diff_primary[_qp] * _grad_u_primary[_qp] * _normals_primary[_qp]);
      }

      return residual;
    }

    default:
      return 0;
  }
}
