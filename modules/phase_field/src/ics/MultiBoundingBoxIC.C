//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiBoundingBoxIC.h"
#include "MooseMesh.h"

registerMooseObject("PhaseFieldApp", MultiBoundingBoxIC);

namespace
{
// Convenience function for sizing a vector to "n" given a vector with size 1 or "n"
std::vector<Real>
sizeVector(std::vector<Real> v, std::size_t size)
{
  if (v.size() == 1)
    return std::vector<Real>(size, v[0]);
  else
    return v;
}
}

InputParameters
MultiBoundingBoxIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription("Specify variable values inside and outside a list of box shaped "
                             "axis-aligned regions defined by pairs of opposing corners");
  params.addRequiredParam<std::vector<Point>>("corners", "The corner coordinates boxes");
  params.addRequiredParam<std::vector<Point>>(
      "opposite_corners", "The coordinates of the opposite corners of the boxes");
  params.addRequiredParam<std::vector<Real>>("inside",
                                             "The value of the variable inside each box "
                                             "(one value per box or a single value for "
                                             "all boxes)");
  params.addParam<Real>("outside", 0.0, "The value of the variable outside the box");

  params.addClassDescription("Allows setting the initial condition of a value of a field inside "
                             "and outside multiple bounding boxes.");
  return params;
}

MultiBoundingBoxIC::MultiBoundingBoxIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _c1(getParam<std::vector<Point>>("corners")),
    _c2(getParam<std::vector<Point>>("opposite_corners")),
    _nbox(_c1.size()),
    _dim(_fe_problem.mesh().dimension()),
    _inside(sizeVector(getParam<std::vector<Real>>("inside"), _nbox)),
    _outside(getParam<Real>("outside"))
{
  // make sure inputs are the same length
  if (_c2.size() != _nbox || _inside.size() != _nbox)
    mooseError("vector inputs must all be the same size");
}

Real
MultiBoundingBoxIC::value(const Point & p)
{
  Real value = _outside;

  for (unsigned int b = 0; b < _nbox; ++b)
  {
    if ((_c1[b](0) < _c2[b](0) && p(0) >= _c1[b](0) && p(0) <= _c2[b](0)) ||
        (_c1[b](0) >= _c2[b](0) && p(0) <= _c1[b](0) && p(0) >= _c2[b](0)))
      if (_dim <= 1 || (_c1[b](1) < _c2[b](1) && p(1) >= _c1[b](1) && p(1) <= _c2[b](1)) ||
          (_c1[b](1) >= _c2[b](1) && p(1) <= _c1[b](1) && p(1) >= _c2[b](1)))
        if (_dim <= 2 || (_c1[b](2) < _c2[b](2) && p(2) >= _c1[b](2) && p(2) <= _c2[b](2)) ||
            (_c1[b](2) >= _c2[b](2) && p(2) <= _c1[b](2) && p(2) >= _c2[b](2)))
        {
          value = _inside[b];
          break;
        }
  }

  return value;
}
