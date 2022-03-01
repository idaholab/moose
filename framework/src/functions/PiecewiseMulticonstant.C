//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseMulticonstant.h"
#include "GriddedData.h"

registerMooseObject("MooseApp", PiecewiseMulticonstant);

InputParameters
PiecewiseMulticonstant::validParams()
{
  InputParameters params = PiecewiseMultiInterpolation::validParams();

  MultiMooseEnum direction("left=0 right=1");
  params.addParam<MultiMooseEnum>(
      "direction", direction, "Direction to look to find value for each interpolation dimension.");

  params.addClassDescription(
      "PiecewiseMulticonstant performs constant interpolation on 1D, 2D, 3D or 4D "
      "data.  The data_file specifies the axes directions and the function "
      "values.  If a point lies outside the data range, the appropriate end "
      "value is used.");
  return params;
}

PiecewiseMulticonstant::PiecewiseMulticonstant(const InputParameters & parameters)
  : PiecewiseMultiInterpolation(parameters), _direction(getParam<MultiMooseEnum>("direction"))
{
  if (_direction.size() != _dim)
    mooseError("Parameter direction must have a size identical to ", _dim);
}

ADReal
PiecewiseMulticonstant::value(const ADReal & t, const ADPoint & p) const
{
  // piecewise constant derivatives are zero everywhere (ignore discontinuities)
  return value(MetaPhysicL::raw_value(t), MetaPhysicL::raw_value(p));
}

Real
PiecewiseMulticonstant::sample(const GridPoint & pt) const
{
  GridIndex left(_dim);
  GridIndex right(_dim);
  GridIndex arg(_dim);
  for (unsigned int i = 0; i < _dim; ++i)
  {
    getNeighborIndices(_grid[i], pt[i], left[i], right[i]);
    if (_direction.get(i) == 0)
      arg[i] = left[i];
    else
      arg[i] = right[i];
  }

  // return the point
  return _gridded_data->evaluateFcn(arg);
}

RealGradient
PiecewiseMulticonstant::gradient(Real, const Point &) const
{
  return 0.0;
}

Real
PiecewiseMulticonstant::timeDerivative(Real, const Point &) const
{
  return 0.0;
}
