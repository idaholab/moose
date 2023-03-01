//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorConstantPointSource.h"

registerMooseObject("MooseApp", VectorConstantPointSource);

InputParameters
VectorConstantPointSource::validParams()
{
  InputParameters params = VectorDiracKernel::validParams();
  params.addClassDescription("Residual contribution of a constant point source term.");
  params.addRequiredParam<RealVectorValue>("values", "The value of the point source");
  params.addRequiredParam<std::vector<Real>>("point", "The x,y,z coordinates of the point");
  params.declareControllable("values");
  return params;
}

VectorConstantPointSource::VectorConstantPointSource(const InputParameters & parameters)
  : VectorDiracKernel(parameters),
    _values(getParam<RealVectorValue>("values")),
    _point_param(getParam<std::vector<Real>>("point"))
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
VectorConstantPointSource::addPoints()
{
  addPoint(_p);
}

Real
VectorConstantPointSource::computeQpResidual()
{
  //  This is negative because it's a forcing function that has been brought over to the left side
  return -_test[_i][_qp] * _values;
}
