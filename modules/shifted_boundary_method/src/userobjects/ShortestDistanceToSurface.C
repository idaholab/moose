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
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredParam<std::vector<FunctionName>>("surfaces",
                                                     "Level-set function for distance.");

  params.addClassDescription("Base distance user object.");
  return params;
}

ShortestDistanceToSurface::ShortestDistanceToSurface(const InputParameters & parameters)
  : ElementUserObject(parameters)
{
  const auto function_names = getParam<std::vector<FunctionName>>("surfaces");
  _distance_functions = SBMUtils::buildDistanceFunctions(function_names, *this);

  if (_distance_functions.empty())
    paramError("surfaces", "ShortestDistanceToSurface requires at least one surface function.");
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

const RealVectorValue
ShortestDistanceToSurface::distanceVectorByFunc(const Point & pt,
                                                Real t,
                                                const Function * func) const
{
  return SBMUtils::distanceVectorFromFunction(func, pt, t);
}

const RealVectorValue
ShortestDistanceToSurface::trueNormalByFunc(const Point & pt, Real t, const Function * func) const
{
  return SBMUtils::trueNormalFromFunction(func, pt, t);
}
