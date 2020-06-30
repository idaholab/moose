//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EqualValueNodalConstraint.h"

registerMooseObject("MooseTestApp", EqualValueNodalConstraint);

InputParameters
EqualValueNodalConstraint::validParams()
{
  InputParameters params = NodalConstraint::validParams();
  params.addRequiredParam<unsigned int>("primary", "The ID of the primary node");
  params.addRequiredParam<unsigned int>("secondary", "The ID of the secondary node");
  params.addRequiredParam<Real>("penalty", "The penalty used for the boundary term");
  return params;
}

EqualValueNodalConstraint::EqualValueNodalConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters), _penalty(getParam<Real>("penalty"))
{
  _connected_nodes.push_back(getParam<unsigned int>("secondary"));
  _primary_node_vector.push_back(getParam<unsigned int>("primary"));
}

EqualValueNodalConstraint::~EqualValueNodalConstraint() {}

Real
EqualValueNodalConstraint::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::Primary:
      return (_u_primary[_j] - _u_secondary[_i]) * _penalty;

    case Moose::Secondary:
      return (_u_secondary[_i] - _u_primary[_j]) * _penalty;
  }

  return 0.;
}

Real
EqualValueNodalConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::PrimaryPrimary:
      return _penalty;

    case Moose::PrimarySecondary:
      return -_penalty;

    case Moose::SecondarySecondary:
      return _penalty;

    case Moose::SecondaryPrimary:
      return -_penalty;

    default:
      return 0;
  }
}
