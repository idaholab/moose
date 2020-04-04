//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "C1ICBase.h"

// Portions of this code Copyright 2007-2009 Roy Stogner
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
//"Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

InputParameters
C1ICBase::validParams()
{
  InputParameters params = InitialCondition::validParams();

  params.addParam<Real>("average", 0, "The average value");
  params.addParam<Real>("amplitude", 1., "The amplitude");
  params.addParam<Real>("length", 0.75, "The length");
  params.addParam<Real>("width", .125, "The width");
  params.addParam<Real>("buffer", 0.03125, "A small area between the max value and the interface");
  params.addParam<Real>("interface", 0.03125, "The interface width");

  return params;
}

C1ICBase::C1ICBase(const InputParameters & parameters)
  : InitialCondition(parameters),
    _average(parameters.get<Real>("average")),
    _amplitude(parameters.get<Real>("amplitude")),
    _length(parameters.get<Real>("length")),
    _width(parameters.get<Real>("width")),
    _buffer(parameters.get<Real>("buffer")),
    _interface(parameters.get<Real>("interface"))
{
}

Number
C1ICBase::interfaceValue(Real r)
{
  Real x = (r - _buffer) / _interface;

  if (x < 0.0)
    return (_average + _amplitude);
  if (x > 1.0)
    return (_average - _amplitude);

  return ((1.0 + 4.0 * x * x * x - 6.0 * x * x) * _amplitude + _average);
}

Number
C1ICBase::interfaceDerivative(Real r)
{
  Real x = (r - _buffer) / _interface;

  if (x < 0.0)
    return 0.0;
  if (x > 1.0)
    return 0.0;

  return ((12.0 * x * x - 12.0 * x) * _amplitude);
}
