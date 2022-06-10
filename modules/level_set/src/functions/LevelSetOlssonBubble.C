//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LevelSetOlssonBubble.h"

registerMooseObject("LevelSetApp", LevelSetOlssonBubble);

InputParameters
LevelSetOlssonBubble::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription("Implementation of 'bubble' ranging from 0 to 1.");
  params.addParam<RealVectorValue>(
      "center", RealVectorValue(0.5, 0.5, 0), "The center of the bubble.");
  params.addParam<Real>("radius", 0.15, "The radius of the bubble.");
  params.addParam<Real>("epsilon", 0.01, "The interface thickness.");
  return params;
}

LevelSetOlssonBubble::LevelSetOlssonBubble(const InputParameters & parameters)
  : Function(parameters),
    _center(getParam<RealVectorValue>("center")),
    _radius(getParam<Real>("radius")),
    _epsilon(getParam<Real>("epsilon"))
{
}

Real
LevelSetOlssonBubble::value(Real /*t*/, const Point & p) const
{
  const auto x = ((p - _center).norm() - _radius) / _epsilon;
  return 1.0 / (1 + std::exp(x));
}

ADReal
LevelSetOlssonBubble::value(const ADReal & /*t*/, const ADPoint & p) const
{
  const auto x = ((p - _center).norm() - _radius) / _epsilon;
  return 1.0 / (1 + std::exp(x));
}

RealGradient
LevelSetOlssonBubble::gradient(Real /*t*/, const Point & p) const
{
  Real norm = (p - _center).norm();
  Real g = (norm - _radius) / _epsilon;
  RealGradient output;

  Real g_prime;
  for (const auto i : make_range(Moose::dim))
  {
    g_prime = (p(i) - _center(i)) / (_epsilon * norm);
    output(i) = -(g_prime * std::exp(g)) / ((std::exp(g) + 1) * (std::exp(g) + 1));
  }
  return output;
}
