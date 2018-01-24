//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantDamper.h"

template <>
InputParameters
validParams<ConstantDamper>()
{
  InputParameters params = validParams<GeneralDamper>();
  params.addRequiredParam<Real>("damping",
                                "The percentage (between 0 and 1) of the newton update to take.");
  return params;
}

ConstantDamper::ConstantDamper(const InputParameters & parameters)
  : GeneralDamper(parameters), _damping(getParam<Real>("damping"))
{
}

Real
ConstantDamper::computeDamping(const NumericVector<Number> & /*solution*/,
                               const NumericVector<Number> & /*update*/)
{
  return _damping;
}
