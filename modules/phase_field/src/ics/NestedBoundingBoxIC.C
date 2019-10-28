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

template <>
InputParameters
validParams<NestedBoundingBoxIC>()
{
  InputParameters params = validParams<SmoothMultiBoundingBoxBaseIC>();
  params.addClassDescription("Specify variable values inside a list of nested boxes shaped "
                             "axis-aligned regions defined by pairs of opposing corners");
  params.addParam<Real>("outside", 0.0, "The value of the variable outside the largest boxes");
  return params;
}

NestedBoundingBoxIC::NestedBoundingBoxIC(const InputParameters & parameters)
  : SmoothMultiBoundingBoxBaseIC(parameters),
    _outside(getParam<Real>("outside"))

        Real NestedBoundingBoxIC::value_assign(const Point & p)
{
  for (unsigned int b = 0; b < _nbox; ++b)
  {
    if (_c1[b](0) < _c2[b](0) && p(0) >= _c1[b](0) - _int_width && p(0) <= _c2[b](0) + _int_width)
      if (_dim <= 1 || (_c1[b](1) < _c2[b](1) && p(1) >= _c1[b](1) - _int_width &&
                        p(1) <= _c2[b](1) + _int_width))
        if (_dim <= 2 || (_c1[b](2) < _c2[b](2) && p(2) >= _c1[b](2) - _int_width &&
                          p(2) <= _c2[b](2) + _int_width))
        {
          Real f_in = 1.0;
          for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
            if (_c1[b](i) != _c2[b](i))
              f_in *= 0.5 * (std::tanh(2.0 * libMesh::PI * (p(i) - _c1[b](i)) / _int_width) -
                             std::tanh(2.0 * libMesh::PI * (p(i) - _c2[b](i)) / _int_width));
          if (b == _nbox - 1)
            value = _outside + (_inside[b] - _outside) * f_in;
          else
            value = _inside[b + 1] + (_inside[b] - _inside[b + 1]) * f_in;
          break;
        }
  }
}
