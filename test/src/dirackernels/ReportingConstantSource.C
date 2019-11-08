//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReportingConstantSource.h"

registerMooseObject("MooseTestApp", ReportingConstantSource);

InputParameters
ReportingConstantSource::validParams()
{
  InputParameters params = DiracKernel::validParams();
  params.addRequiredParam<std::vector<Real>>("point", "The x,y,z coordinates of the point");
  params.addRequiredCoupledVar("shared", "Constant auxilary variable for storing the total flux");
  params.addParam<Real>("factor", 1, "The multiplier for the shared source value");
  return params;
}

ReportingConstantSource::ReportingConstantSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    _shared_var(coupledScalarValue("shared")),
    _point_param(getParam<std::vector<Real>>("point")),
    _factor(getParam<Real>("factor"))
{
  _p(0) = _point_param[0];

  if (_point_param.size() > 1)
  {
    _p(1) = _point_param[1];

    if (_point_param.size() > 2)
      _p(2) = _point_param[2];
  }
}

void
ReportingConstantSource::addPoints()
{
  addPoint(_p);
}

Real
ReportingConstantSource::computeQpResidual()
{
  // This is negative because it's a forcing function that has been brought over to the left side.
  return -_test[_i][_qp] * _shared_var[0] * _factor;
}
