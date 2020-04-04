//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmoothMultiBoundingBoxBaseIC.h"
#include "MooseMesh.h"

InputParameters
SmoothMultiBoundingBoxBaseIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredParam<Real>("outside", "Outside value");
  params.addRequiredParam<std::vector<Point>>(
      "smaller_coordinate_corners",
      "For 1D, these are the left points; for 2D, these are the bottom left corners; for 3D, these "
      "are the back bottom left corners. The format is (corner1_x corner1_y corner1_z corner2_x "
      "corner2_y corner2_z ...)");
  params.addRequiredParam<std::vector<Point>>(
      "larger_coordinate_corners",
      "For 1D, these are the right points; for 2D, these are the top right corners; for 3D, these "
      "are the front top right corners. The format is (corner1_x corner1_y corner1_z corner2_x "
      "corner2_y corner2_z ...)");
  params.addRequiredParam<Real>("int_width", "The value of the interfacial width between boxes");
  params.addRequiredParam<std::vector<Real>>("inside",
                                             "The value of the variable inside each box "
                                             "(one value per box or a single value for "
                                             "all boxes)");
  return params;
}

SmoothMultiBoundingBoxBaseIC::SmoothMultiBoundingBoxBaseIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _outside(getParam<Real>("outside")),
    _c1(getParam<std::vector<Point>>("smaller_coordinate_corners")),
    _c2(getParam<std::vector<Point>>("larger_coordinate_corners")),
    _nbox(_c1.size()),
    _int_width(getParam<Real>("int_width")),
    _dim(_fe_problem.mesh().dimension()),
    _inside(getParam<std::vector<Real>>("inside"))
{
}

Real
SmoothMultiBoundingBoxBaseIC::value(const Point & p)
{
  Real value = _outside;

  // if "inside" vector only has size 1, all the boxes have the same inside value
  if (_inside.size() == 1)
  {
    _inside.assign(_nbox, _inside[0]);
  }

  // make sure inputs are the same length
  if (_c2.size() != _nbox || _inside.size() != _nbox)
    paramError("vector inputs must all be the same size");

  if (_int_width < 0.0)
    paramError("'int_width' should be non-negative");

  if (_int_width == 0.0)
  {
    for (unsigned int b = 0; b < _nbox; ++b)
    {
      for (unsigned int i = 0; i < _dim; ++i)
        if (_c1[b](i) < _c2[b](i) && p(i) >= _c1[b](i) && p(i) <= _c2[b](i))
        {
          if (i != _dim - 1)
            continue;
          value = _inside[b];
          break;
        }
        else
          break;
    }
  }
  return value;
}
