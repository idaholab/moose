//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrossIC.h"

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

registerMooseObject("PhaseFieldApp", CrossIC);

InputParameters
CrossIC::validParams()
{
  InputParameters params = C1ICBase::validParams();
  params.addClassDescription("Cross-shaped initial condition");
  params.addParam<Real>("x1", 0.0, "The x coordinate of the lower left-hand corner of the box");
  params.addParam<Real>("y1", 0.0, "The y coordinate of the lower left-hand corner of the box");
  params.addParam<Real>("x2", 1.0, "The x coordinate of the upper right-hand corner of the box");
  params.addParam<Real>("y2", 1.0, "The y coordinate of the upper right-hand corner of the box");
  return params;
}

CrossIC::CrossIC(const InputParameters & parameters)
  : C1ICBase(parameters),
    _x1(parameters.get<Real>("x1")),
    _y1(parameters.get<Real>("y1")),
    _x2(parameters.get<Real>("x2")),
    _y2(parameters.get<Real>("y2"))
{
}

Real
CrossIC::value(const Point & p)
{
  const Real x = (p(0) - _x1) / (_x2 - _x1);
  const Real y = (p(1) - _y1) / (_y2 - _y1);

  const Real cmax = _average + _amplitude;
  const Real cmin = _average - _amplitude;

  if (x > (0.5 + _length / 2.0 + _buffer + _interface))
  {
    return cmin;
  }
  else if (x > 0.5 + _length / 2.0)
  {
    if (y > 0.5 + _width / 2.0 + _buffer + _interface)
    {
      return cmin;
    }
    else if (y > 0.5 + _width / 2.0)
    {
      Real xd = x - .5 - _length / 2.0;
      Real yd = y - .5 - _width / 2.0;
      Real r = std::sqrt(xd * xd + yd * yd);
      return interfaceValue(r);
    }
    else if (y > 0.5 - _width / 2.0)
    {
      return interfaceValue(x - .5 - _length / 2.0);
    }
    else if (y > 0.5 - _width / 2. - _buffer - _interface)
    {
      Real xd = x - .5 - _length / 2.0;
      Real yd = y - .5 + _width / 2.0;
      Real r = std::sqrt(xd * xd + yd * yd);
      return interfaceValue(r);
    }
    else
    {
      return cmin;
    }
  }
  else if (x > 0.5 + _width / 2. + 2. * _buffer + _interface)
  {
    if (y > 0.5 + _width / 2 + _buffer + _interface)
      return cmin;
    else if (y > 0.5 + _width / 2.0)
      return interfaceValue(y - .5 - _width / 2.0);
    else if (y > 0.5 - _width / 2.0)
      return cmax;
    else if (y > 0.5 - _width / 2. - _buffer - _interface)
      return interfaceValue(.5 - _width / 2. - y);
    else
      return cmin;
  }
  else if (x > 0.5 + _width / 2.0)
  {
    if (y > 0.5 + _length / 2. + _buffer + _interface)
    {
      return cmin;
    }
    else if (y > 0.5 + _length / 2.0)
    {
      Real xd = x - (.5 + _width / 2.0);
      Real yd = y - (.5 + _length / 2.0);
      Real r = std::sqrt(xd * xd + yd * yd);
      return interfaceValue(r);
    }
    else if (y > 0.5 + _width / 2. + 2. * _buffer + _interface)
    {
      return interfaceValue(x - .5 - _width / 2.0);
    }
    else if (y > 0.5 + _width / 2.0)
    {
      Real xd = x - (.5 + _width / 2. + 2. * _buffer + _interface);
      Real yd = y - (.5 + _width / 2. + 2. * _buffer + _interface);
      Real r = std::sqrt(xd * xd + yd * yd);
      return interfaceValue(2. * _buffer + _interface - r);
    }
    else if (y > 0.5 - _width / 2.0)
    {
      return cmax;
    }
    else if (y > 0.5 - _width / 2. - 2. * _buffer - _interface)
    {
      Real xd = x - (.5 + _width / 2. + 2. * _buffer + _interface);
      Real yd = y - (.5 - _width / 2. - 2. * _buffer - _interface);
      Real r = std::sqrt(xd * xd + yd * yd);
      return interfaceValue(2. * _buffer + _interface - r);
    }
    else if (y > 0.5 - _length / 2.0)
    {
      return interfaceValue(x - .5 - _width / 2.0);
    }
    else if (y > 0.5 - _length / 2. - _buffer - _interface)
    {
      Real xd = x - (.5 + _width / 2.0);
      Real yd = y - (.5 - _length / 2.0);
      Real r = std::sqrt(xd * xd + yd * yd);
      return interfaceValue(r);
    }
    else
    {
      return cmin;
    }
  }
  else if (x > 0.5 - _width / 2.0)
  {
    if (y > 0.5 + _length / 2 + _buffer + _interface)
      return cmin;
    else if (y > 0.5 + _length / 2.0)
      return interfaceValue(y - .5 - _length / 2.0);
    else if (y > 0.5 - _length / 2.0)
      return cmax;
    else if (y > 0.5 - _length / 2. - _buffer - _interface)
      return interfaceValue(.5 - _length / 2. - y);
    else
      return cmin;
  }
  else if (x > 0.5 - _width / 2. - 2. * _buffer - _interface)
  {
    if (y > 0.5 + _length / 2. + _buffer + _interface)
    {
      return cmin;
    }
    else if (y > 0.5 + _length / 2.0)
    {
      Real xd = x - (.5 - _width / 2.0);
      Real yd = y - (.5 + _length / 2.0);
      Real r = std::sqrt(xd * xd + yd * yd);
      return interfaceValue(r);
    }
    else if (y > 0.5 + _width / 2. + 2. * _buffer + _interface)
    {
      return interfaceValue(.5 - _width / 2. - x);
    }
    else if (y > 0.5 + _width / 2.0)
    {
      Real xd = x - (.5 - _width / 2. - 2. * _buffer - _interface);
      Real yd = y - (.5 + _width / 2. + 2. * _buffer + _interface);
      Real r = std::sqrt(xd * xd + yd * yd);
      return interfaceValue(2. * _buffer + _interface - r);
    }
    else if (y > 0.5 - _width / 2.0)
    {
      return cmax;
    }
    else if (y > 0.5 - _width / 2. - 2. * _buffer - _interface)
    {
      Real xd = x - (.5 - _width / 2. - 2. * _buffer - _interface);
      Real yd = y - (.5 - _width / 2. - 2. * _buffer - _interface);
      Real r = std::sqrt(xd * xd + yd * yd);
      return interfaceValue(2. * _buffer + _interface - r);
    }
    else if (y > 0.5 - _length / 2.0)
    {
      return interfaceValue(.5 - _width / 2. - x);
    }
    else if (y > 0.5 - _length / 2. - _buffer - _interface)
    {
      Real xd = x - (.5 - _width / 2.0);
      Real yd = y - (.5 - _length / 2.0);
      Real r = std::sqrt(xd * xd + yd * yd);
      return interfaceValue(r);
    }
    else
    {
      return cmin;
    }
  }
  else if (x > 0.5 - _length / 2.0)
  {
    if (y > 0.5 + _width / 2 + _buffer + _interface)
      return cmin;
    else if (y > 0.5 + _width / 2.0)
      return interfaceValue(y - .5 - _width / 2.0);
    else if (y > 0.5 - _width / 2.0)
      return cmax;
    else if (y > 0.5 - _width / 2. - _buffer - _interface)
      return interfaceValue(.5 - _width / 2. - y);
    else
      return cmin;
  }
  else if (x > (.5 - _length / 2. - _buffer - _interface))
  {
    if (y > 0.5 + _width / 2. + _buffer + _interface)
    {
      return cmin;
    }
    else if (y > 0.5 + _width / 2.0)
    {
      Real xd = x - (.5 - _length / 2.0);
      Real yd = y - .5 - _width / 2.0;
      Real r = std::sqrt(xd * xd + yd * yd);
      return interfaceValue(r);
    }
    else if (y > 0.5 - _width / 2.0)
    {
      return interfaceValue(.5 - _length / 2. - x);
    }
    else if (y > 0.5 - _width / 2. - _buffer - _interface)
    {
      Real xd = x - (.5 - _length / 2.0);
      Real yd = y - .5 + _width / 2.0;
      Real r = std::sqrt(xd * xd + yd * yd);
      return interfaceValue(r);
    }
    else
      return cmin;
  }
  else
    return cmin;
}

RealGradient
CrossIC::gradient(const Point & p)
{
  Point pxminus = p, pxplus = p, pyminus = p, pyplus = p;

  pxminus(0) -= TOLERANCE;
  pyminus(1) -= TOLERANCE;
  pxplus(0) += TOLERANCE;
  pyplus(1) += TOLERANCE;

  Number uxminus = value(pxminus), uxplus = value(pxplus), uyminus = value(pyminus),
         uyplus = value(pyplus);

  return Gradient((uxplus - uxminus) / 2.0 / TOLERANCE, (uyplus - uyminus) / 2.0 / TOLERANCE);
}
