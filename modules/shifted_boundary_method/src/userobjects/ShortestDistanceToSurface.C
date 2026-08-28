//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShortestDistanceToSurface.h"
#include "SBMUtils.h"

registerMooseObject("ShiftedBoundaryMethodApp", ShortestDistanceToSurface);

InputParameters
ShortestDistanceToSurface::validParams()
{
  InputParameters params = ThreadedGeneralUserObject::validParams();
  params.addRequiredParam<std::vector<FunctionName>>(
      "surfaces",
      "Functions defining the surfaces to measure distance to; each must be a ParsedFunction "
      "level set or an UnsignedDistanceToSurfaceMesh.");
  params.addParam<bool>(
      "signed_distance", false, "Whether the distance functions are signed or unsigned.");
  params.addClassDescription(
      "Computes the shortest distance vector and true normal to one or more surfaces defined by "
      "parsed level-set functions and/or mesh-based distance functions.");
  return params;
}

ShortestDistanceToSurface::ShortestDistanceToSurface(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters), _signed_distance(getParam<bool>("signed_distance"))
{
  const auto function_names = getParam<std::vector<FunctionName>>("surfaces");
  _distance_functions = SBMUtils::buildDistanceFunctions(function_names, *this);
}

Real
ShortestDistanceToSurface::value(Real t, const Point & p) const
{
  if (!_signed_distance)
    return distanceVector(p).norm();

  return SBMUtils::unionSignedDistance(_distance_functions, t, p);
}

RealVectorValue
ShortestDistanceToSurface::distanceVector(const Point & pt) const
{
  return SBMUtils::closestDistanceVector(_distance_functions, pt, _t);
}

RealVectorValue
ShortestDistanceToSurface::trueNormal(const Point & pt) const
{
  return SBMUtils::closestTrueNormalVector(_distance_functions, pt, _t);
}

RealVectorValue
ShortestDistanceToSurface::distanceVectorByIndex(unsigned int idx, const Point & pt) const
{
  const auto & func = _distance_functions.at(idx);
  return SBMUtils::distanceVectorFromFunction(func, pt, _t);
}

RealVectorValue
ShortestDistanceToSurface::trueNormalByIndex(unsigned int idx, const Point & pt) const
{
  const auto & func = _distance_functions.at(idx);
  return SBMUtils::trueNormalFromFunction(func, pt, _t);
}

RealVectorValue
ShortestDistanceToSurface::distanceVectorByFunc(const Point & pt,
                                                Real t,
                                                const Function * func) const
{
  return SBMUtils::distanceVectorFromFunction(func, pt, t);
}

RealVectorValue
ShortestDistanceToSurface::trueNormalByFunc(const Point & pt, Real t, const Function * func) const
{
  return SBMUtils::trueNormalFromFunction(func, pt, t);
}
