//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpatiallyVaryingSource.h"
#include "Function.h"

registerMooseObject("MooseTestApp", SpatiallyVaryingSource);

InputParameters
SpatiallyVaryingSource::validParams()
{
  return ADMortarConstraint::validParams();
}

SpatiallyVaryingSource::SpatiallyVaryingSource(const InputParameters & parameters)
  : ADMortarConstraint(parameters)
{
}

ADReal
SpatiallyVaryingSource::computeQpResidual(Moose::MortarType type)
{
  const auto tangent = _normals[_qp].cross(VectorValue<Real>(0, 0, 1));

  switch (type)
  {
    case Moose::MortarType::Secondary:
      return _q_point[_qp] * tangent * _test_secondary[_i][_qp];

    case Moose::MortarType::Primary:
      return _q_point[_qp] * tangent * _test_primary[_i][_qp];

    default:
      mooseError("Don't call me, I'm not involved in this physics.");
  }
}
