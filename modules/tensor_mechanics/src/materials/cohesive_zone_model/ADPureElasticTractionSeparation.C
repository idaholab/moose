//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPureElasticTractionSeparation.h"
#include "PureElasticTractionSeparation.h"

registerMooseObject("TensorMechanicsApp", ADPureElasticTractionSeparation);

InputParameters
ADPureElasticTractionSeparation::validParams()
{
  InputParameters params = PureElasticTractionSeparation::validParams();
  return params;
}

ADPureElasticTractionSeparation::ADPureElasticTractionSeparation(const InputParameters & parameters)
  : ADCZMComputeLocalTractionTotalBase(parameters),
    _K(std::vector<Real>{getParam<Real>("normal_stiffness"),
                         getParam<Real>("tangent_stiffness"),
                         getParam<Real>("tangent_stiffness")})
{
}

void
ADPureElasticTractionSeparation::computeInterfaceTraction()
{
  _interface_traction[_qp] = _K * _interface_displacement_jump[_qp];
}
