//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NestedBoundingBoxIC.h"
#include "MooseMesh.h"

registerMooseObject("PhaseFieldApp", NestedBoundingBoxIC);

InputParameters
NestedBoundingBoxIC::validParams()
{
  InputParameters params = SmoothMultiBoundingBoxBaseIC::validParams();
  params.addClassDescription("Specify variable values inside a list of nested boxes shaped "
                             "axis-aligned regions defined by pairs of opposing corners");
  params.addParam<Real>("outside", 0.0, "The value of the variable outside the largest boxes");
  return params;
}

NestedBoundingBoxIC::NestedBoundingBoxIC(const InputParameters & parameters)
  : SmoothMultiBoundingBoxBaseIC(parameters), _outside(getParam<Real>("outside"))
{
}

Real
NestedBoundingBoxIC::value(const Point & p)
{
  Real value = SmoothMultiBoundingBoxBaseIC::value(p);

  if (_int_width != 0.0)
  {
    for (unsigned int b = 0; b < _nbox; ++b)
    {
      for (unsigned int i = 0; i < _dim; ++i)
      {
        if (_c1[b](i) < _c2[b](i) && p(i) >= _c1[b](i) - _int_width &&
            p(i) <= _c2[b](i) + _int_width)
        {
          if (i != _dim - 1)
            continue;
          Real f_in = 1.0;
          for (unsigned int j = 0; j < _dim; ++j)
            f_in *= 0.5 * (std::tanh(2.0 * libMesh::pi * (p(j) - _c1[b](j)) / _int_width) -
                           std::tanh(2.0 * libMesh::pi * (p(j) - _c2[b](j)) / _int_width));
          if (b == _nbox - 1)
            value = _outside + (_inside[b] - _outside) * f_in;
          else
            value = _inside[b + 1] + (_inside[b] - _inside[b + 1]) * f_in;
          goto label;
        }
        else if (_c1[b](i) >= _c2[b](i))
          mooseError("The coordinates of the smaller_coordinate_corners are equal to or larger "
                     "than that of "
                     "the larger_coordinate_corners.");
        else
          break;
      }
    }
  }
label:
  return value;
}
