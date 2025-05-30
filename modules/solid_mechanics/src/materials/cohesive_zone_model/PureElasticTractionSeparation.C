//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PureElasticTractionSeparation.h"

registerMooseObject("SolidMechanicsApp", PureElasticTractionSeparation);

InputParameters
PureElasticTractionSeparation::validParams()
{
  InputParameters params = CZMComputeLocalTractionTotalBase::validParams();
  params.addClassDescription("Pure elastic traction separation law.");
  params.addRequiredParam<Real>("normal_stiffness", "The value of K in the normal direction");
  params.addRequiredParam<Real>("tangent_stiffness", "The value of K in the tangent direction");
  return params;
}

PureElasticTractionSeparation::PureElasticTractionSeparation(const InputParameters & parameters)
  : CZMComputeLocalTractionTotalBase(parameters),
    _K(std::vector<Real>{getParam<Real>("normal_stiffness"),
                         getParam<Real>("tangent_stiffness"),
                         getParam<Real>("tangent_stiffness")})
{
}

void
PureElasticTractionSeparation::computeInterfaceTractionAndDerivatives()
{
  _interface_traction[_qp] = _K * _interface_displacement_jump[_qp];
  _dinterface_traction_djump[_qp] = _K;
}
