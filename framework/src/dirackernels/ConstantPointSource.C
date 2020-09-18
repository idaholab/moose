//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantPointSource.h"

registerMooseObject("MooseApp", ConstantPointSource);

defineLegacyParams(ConstantPointSource);

InputParameters
ConstantPointSource::validParams()
{
  InputParameters params = DiracKernel::validParams();

  params.addParam<std::vector<Real>>("value", "The values of the point sources");
  params.addParam<std::vector<Real>>("point", "The x,y,z coordinates of the points");
  params.declareControllable("value");
  return params;
}

ConstantPointSource::ConstantPointSource(const InputParameters & parameters)
  : DiracKernel(parameters)
{
  const std::vector<Real> value = getParam<std::vector<Real>>("value");
  const std::vector<Real> point_param = getParam<std::vector<Real>>("point");
  size_t dim = point_param.size() / value.size();
  Point pt;
  _point_to_value.clear();
  for (size_t i = 0; i < value.size(); ++i)
  {
    pt(0) = point_param[dim * i + 0];
    if (dim > 1)
    {
      pt(1) = point_param[dim * i + 1];
      if (dim > 2)
      {
        pt(2) = point_param[dim * i + 2];
      }
    }
    _point_to_value[pt] = value[i];
  }
}

void
ConstantPointSource::addPoints()
{
  unsigned index = 0;
  for (auto & point : _point_to_value)
  {
    addPoint(point.first);
    ++index;
  }
}

Real
ConstantPointSource::computeQpResidual()
{
  //  This is negative because it's a forcing function that has been brought over to the left side
  return -_test[_i][_qp] * _point_to_value[_current_point];
}
