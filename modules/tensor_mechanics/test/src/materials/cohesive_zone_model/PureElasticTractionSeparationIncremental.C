//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PureElasticTractionSeparationIncremental.h"

registerMooseObject("TensorMechanicsTestApp", PureElasticTractionSeparationIncremental);

InputParameters
PureElasticTractionSeparationIncremental::validParams()
{
  InputParameters params = CZMComputeLocalTractionIncrementalBase::validParams();
  params.addClassDescription("Pure Elastic, Incremental traction separation law.");
  params.addRequiredParam<Real>("normal_stiffness", "The value of K in the normal direction");
  params.addRequiredParam<Real>("tangent_stiffness", "The value of K in the tangent direction");
  return params;
}

PureElasticTractionSeparationIncremental::PureElasticTractionSeparationIncremental(
    const InputParameters & parameters)
  : CZMComputeLocalTractionIncrementalBase(parameters),
    _K(std::vector<Real>{getParam<Real>("normal_stiffness"),
                         getParam<Real>("tangent_stiffness"),
                         getParam<Real>("tangent_stiffness")})
{
}

void
PureElasticTractionSeparationIncremental::computeInterfaceTractionIncrementAndDerivatives()
{
  _interface_traction_inc[_qp] = _K * _interface_displacement_jump_inc[_qp];
  _dinterface_traction_djump[_qp] = _K;
}
