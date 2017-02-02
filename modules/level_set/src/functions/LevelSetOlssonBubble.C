/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// MOOSE includes
#include "LevelSetOlssonBubble.h"

template<>
InputParameters
validParams<LevelSetOlssonBubble>()
{
  InputParameters params = validParams<Function>();
  params.addClassDescription("Implementation of 'bubble' ranging from 0 to 1.");
  params.addParam<RealVectorValue>("center", RealVectorValue(0.5, 0.5, 0), "The center of the bubble.");
  params.addParam<Real>("radius", 0.15, "The radius of the bubble.");
  params.addParam<Real>("epsilon", 0.01, "The interface thickness.");
  return params;
}

LevelSetOlssonBubble::LevelSetOlssonBubble(const InputParameters & parameters):
    Function(parameters),
    _center(getParam<RealVectorValue>("center")),
    _radius(getParam<Real>("radius")),
    _epsilon(getParam<Real>("epsilon"))
{
}

Real
LevelSetOlssonBubble::value(Real /*t*/, const Point & p)
{
  const Real x = ((p - _center).size() - _radius) / _epsilon;
  return 1.0 / (1 + std::exp(x));
}

RealGradient
LevelSetOlssonBubble::gradient(Real /*t*/, const Point & p)
{
  Real norm = (p - _center).size();
  Real g = (norm - _radius) / _epsilon;
  RealGradient output;

  Real g_prime;
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    g_prime = (p(i) - _center(i)) / (_epsilon * norm);
    output(i) = (g_prime * exp(g)) / ((exp(g) + 1) * (exp(g) + 1));
  }
  return output;
}
