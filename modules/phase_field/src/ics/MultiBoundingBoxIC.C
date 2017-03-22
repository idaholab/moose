/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MultiBoundingBoxIC.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<MultiBoundingBoxIC>()
{
  InputParameters params = validParams<InitialCondition>();
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
  return params;
}

MultiBoundingBoxIC::MultiBoundingBoxIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _c1(getParam<std::vector<Point>>("corners")),
    _c2(getParam<std::vector<Point>>("opposite_corners")),
    _nbox(_c1.size()),
    _dim(_fe_problem.mesh().dimension()),
    _inside(getParam<std::vector<Real>>("inside")),
    _outside(getParam<Real>("outside"))
{
  // we allow passing in a single value used on the inside of all boxes
  if (_inside.size() == 1)
    _inside.assign(_nbox, _inside[0]);

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
