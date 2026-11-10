//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimestepSizeExceptionMaterial.h"

registerMooseObject("MooseTestApp", TimestepSizeExceptionMaterial);

InputParameters
TimestepSizeExceptionMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<Real>(
      "max_dt", "Maximum timestep size before this test material throws a MooseException.");

  params.addClassDescription(
      "Test material that throws a recoverable MooseException when the timestep is too large.");

  return params;
}

TimestepSizeExceptionMaterial::TimestepSizeExceptionMaterial(const InputParameters & parameters)
  : Material(parameters),
    _property(declareProperty<Real>("exception_test_property")),
    _max_dt(getParam<Real>("max_dt"))
{
}

void
TimestepSizeExceptionMaterial::computeQpProperties()
{
  if (_dt > _max_dt)
    mooseException("TimestepSizeExceptionMaterial failed on timestep ",
                   _t_step,
                   " at time ",
                   _t,
                   " with dt ",
                   _dt);

  _property[_qp] = 1.0;
}
