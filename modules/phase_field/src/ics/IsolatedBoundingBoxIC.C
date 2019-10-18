//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IsolatedBoundingBoxIC.h"
#include "MooseMesh.h"
#include <iostream>

registerMooseObject("PhaseFieldApp", IsolatedBoundingBoxIC);

template <>
InputParameters
validParams<IsolatedBoundingBoxIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addClassDescription(
      "Specify variable values inside and outside a list of isolated boxes shaped "
      "axis-aligned regions defined by pairs of opposing corners");
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
  params.addRequiredParam<std::vector<Real>>("inside",
                                             "The value of the variable inside each box "
                                             "(one value per box)");

  params.addParam<Real>("outside", 0.0, "The value of the variable outside the boxes");

  params.addParam<Real>(
      "int_width", 0.0, "The width of the diffuse interface. Set to 0 for sharp interface.");

  params.addClassDescription("Allows setting the initial condition of the value of a field inside "
                             "multiple isolated bounding boxes.");
  return params;
}

IsolatedBoundingBoxIC::IsolatedBoundingBoxIC(const InputParameters & parameters)
  : MultiBoundingBoxIC(parameters),
    _c1(getParam<std::vector<Point>>("smaller_coordinate_corners")),
    _c2(getParam<std::vector<Point>>("larger_coordinate_corners")),
    _nbox(_c1.size()),
    _int_width(getParam<Real>("int_width")),
    _dim(_fe_problem.mesh().dimension()),
    _inside(getParam<std::vector<Real>>("inside")),
    _outside(getParam<Real>("outside"))

        Real IsolatedBoundingBoxIC::value(const Point & p)
{
  Real value = _outside;

  // if "inside" vector only has size 1, all the boxes have the same inside value
  if (_inside.size() == 1)
    _inside = _inside.assign(_nbox, _inside[0]);

  // make sure inputs are the same length
  if (_c2.size() != _nbox || _inside.size() != _nbox)
    paramError("vector inputs must all be the same size");

  if (_int_width < 0.0)
    paramError("'int_width' should be non-negative");

  if (_int_width == 0.0)
  {
    for (unsigned int b = 0; b < _nbox; ++b)
    {
      if (_c1[b](0) < _c2[b](0) && p(0) >= _c1[b](0) && p(0) <= _c2[b](0))
        if (_dim <= 1 || (_c1[b](1) < _c2[b](1) && p(1) >= _c1[b](1) && p(1) <= _c2[b](1)))
          if (_dim <= 2 || (_c1[b](2) < _c2[b](2) && p(2) >= _c1[b](2) && p(2) <= _c2[b](2)))
          {
            value = _inside[b];
            break;
          }
    }
  }

  else
  {
    for (unsigned int b = 0; b < _nbox; ++b)
    {
      if (_c1[b](0) < _c2[b](0) && p(0) >= _c1[b](0) - _int_width && p(0) <= _c2[b](0) + _int_width)
        if (_dim <= 1 || (_c1[b](1) < _c2[b](1) && p(1) >= _c1[b](1) - _int_width &&
                          p(1) <= _c2[b](1) + _int_width))
          if (_dim <= 2 || (_c1[b](2) < _c2[b](2) && p(2) >= _c1[b](2) - _int_width &&
                            p(2) <= _c2[b](2) + _int_width))
          {
            for (unsigned int n = b + 1; n < _nbox; ++n)
            {
              if (p(0) >= _c1[n](0) - _int_width && p(0) <= _c2[n](0) + _int_width)
                if (_dim <= 1 || (p(1) >= _c1[n](1) - _int_width && p(1) <= _c2[n](1) + _int_width))
                  if (_dim <= 2 ||
                      (p(2) >= _c1[n](2) - _int_width && p(2) <= _c2[n](2) + _int_width))
                  {
                    mooseError("Partially overlapping boxes are not allowed. Note that this "
                               "includes the overlapping diffused interfaces. For nested boxes, "
                               "use NestedBoundingBoxIC.C");
                  }
            }

            Real f_in = 1.0;
            for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
              if (_c1[b](i) != _c2[b](i))
                f_in *= 0.5 * (std::tanh(2.0 * libMesh::PI * (p(i) - _c1[b](i)) / _int_width) -
                               std::tanh(2.0 * libMesh::PI * (p(i) - _c2[b](i)) / _int_width));

            value = _outside + (_inside[b] - _outside) * f_in;
            break;
          }
    }
  }

  return value;
}
