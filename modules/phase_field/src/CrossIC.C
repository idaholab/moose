#include "CrossIC.h"

//Portions of this code Copyright 2007-2009 Roy Stogner
//
//Permission is hereby granted, free of charge, to any person obtaining
//a copy of this software and associated documentation files (the
//"Software"), to deal in the Software without restriction, including
//without limitation the rights to use, copy, modify, merge, publish,
//distribute, sublicense, and/or sell copies of the Software, and to
//permit persons to whom the Software is furnished to do so, subject to
//the following conditions:
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

template<>
InputParameters validParams<CrossIC>()
{
  return validParams<C1ICBase>();
}

CrossIC::CrossIC(std::string name,
                 MooseSystem & moose_system,
                 InputParameters parameters)
  :C1ICBase(name, moose_system, parameters)
{}

Real
CrossIC::value(const Point & p)
{
  Real x = p(0), y = p(1);

  Real cmax = _average + _amplitude;
  Real cmin = _average - _amplitude;

  if (x > (.5 + _length/2. + _buffer + _interface))
  {
    return cmin;
  }
  else if (x > .5 + _length/2.)
  {
    if (y > .5 + _width/2. + _buffer + _interface)
    {
      return cmin;
    }
    else if (y > .5 + _width/2.)
    {
      Real xd = x - .5 - _length/2.;
      Real yd = y - .5 - _width/2.;
      Real r = std::sqrt(xd*xd+yd*yd);
      return interfaceValue(r);
    }
    else if (y > .5 - _width/2.)
    {
      return interfaceValue(x - .5 - _length/2.);
    }
    else if (y > .5 - _width/2. - _buffer - _interface)
    {
      Real xd = x - .5 - _length/2.;
      Real yd = y - .5 + _width/2.;
      Real r = std::sqrt(xd*xd+yd*yd);
      return interfaceValue(r);
    }
    else
    {
      return cmin;
    }
  }
  else if (x > .5 + _width/2. + 2.*_buffer + _interface)
  {
    if (y > .5 + _width/2 + _buffer + _interface)
    {
      return cmin;
    }
    else if (y > .5 + _width/2.)
    {
      return interfaceValue(y - .5 - _width/2.);
    }
    else if (y > .5 - _width/2.)
    {
      return cmax;
    }
    else if (y > .5 - _width/2. - _buffer - _interface)
    {
      return interfaceValue(.5 - _width/2. - y);
    }
    else
    {
      return cmin;
    }
  }
  else if (x > .5 + _width/2.)
  {
    if (y > .5 + _length/2. + _buffer + _interface)
    {
      return cmin;
    }
    else if (y > .5 + _length/2.)
    {
      Real xd = x - (.5 + _width/2.);
      Real yd = y - (.5 + _length/2.);
      Real r = std::sqrt(xd*xd+yd*yd);
      return interfaceValue(r);
    }
    else if (y > .5 + _width/2. + 2.*_buffer + _interface)
    {
      return interfaceValue(x - .5 - _width/2.);
    }
    else if (y > .5 + _width/2.)
    {
      Real xd = x - (.5 + _width/2. + 2.*_buffer + _interface);
      Real yd = y - (.5 + _width/2. + 2.*_buffer + _interface);
      Real r = std::sqrt(xd*xd+yd*yd);
      return interfaceValue(2.*_buffer + _interface - r);
    }
    else if (y > .5 - _width/2.)
    {
      return cmax;
    }
    else if (y > .5 - _width/2. - 2.*_buffer - _interface)
    {
      Real xd = x - (.5 + _width/2. + 2.*_buffer + _interface);
      Real yd = y - (.5 - _width/2. - 2.*_buffer - _interface);
      Real r = std::sqrt(xd*xd+yd*yd);
      return interfaceValue(2.*_buffer + _interface - r);
    }
    else if (y > .5 - _length/2.)
    {
      return interfaceValue(x - .5 - _width/2.);
    }
    else if (y > .5 - _length/2. - _buffer - _interface)
    {
      Real xd = x - (.5 + _width/2.);
      Real yd = y - (.5 - _length/2.);
      Real r = std::sqrt(xd*xd+yd*yd);
      return interfaceValue(r);
    }
    else
    {
      return cmin;
    }
  }
  else if (x > .5 - _width/2.)
  {
    if (y > .5 + _length/2 + _buffer + _interface)
    {
      return cmin;
    }
    else if (y > .5 + _length/2.)
    {
      return interfaceValue(y - .5 - _length/2.);
    }
    else if (y > .5 - _length/2.)
    {
      return cmax;
    }
    else if (y > .5 - _length/2. - _buffer - _interface)
    {
      return interfaceValue(.5 - _length/2. - y);
    }
    else
    {
      return cmin;
    }
  }
  else if (x > .5 - _width/2. - 2.*_buffer - _interface)
  {
    if (y > .5 + _length/2. + _buffer + _interface)
    {
      return cmin;
    }
    else if (y > .5 + _length/2.)
    {
      Real xd = x - (.5 - _width/2.);
      Real yd = y - (.5 + _length/2.);
      Real r = std::sqrt(xd*xd+yd*yd);
      return interfaceValue(r);
    }
    else if (y > .5 + _width/2. + 2.*_buffer + _interface)
    {
      return interfaceValue(.5 - _width/2. - x);
    }
    else if (y > .5 + _width/2.)
    {
      Real xd = x - (.5 - _width/2. - 2.*_buffer - _interface);
      Real yd = y - (.5 + _width/2. + 2.*_buffer + _interface);
      Real r = std::sqrt(xd*xd+yd*yd);
      return interfaceValue(2.*_buffer + _interface - r);
    }
    else if (y > .5 - _width/2.)
    {
      return cmax;
    }
    else if (y > .5 - _width/2. - 2.*_buffer - _interface)
    {
      Real xd = x - (.5 - _width/2. - 2.*_buffer - _interface);
      Real yd = y - (.5 - _width/2. - 2.*_buffer - _interface);
      Real r = std::sqrt(xd*xd+yd*yd);
      return interfaceValue(2.*_buffer + _interface - r);
    }
    else if (y > .5 - _length/2.)
    {
      return interfaceValue(.5 - _width/2. - x);
    }
    else if (y > .5 - _length/2. - _buffer - _interface)
    {
      Real xd = x - (.5 - _width/2.);
      Real yd = y - (.5 - _length/2.);
      Real r = std::sqrt(xd*xd+yd*yd);
      return interfaceValue(r);
    }
    else
    {
      return cmin;
    }
  }
  else if (x > .5 - _length/2.)
  {
    if (y > .5 + _width/2 + _buffer + _interface)
    {
      return cmin;
    }
    else if (y > .5 + _width/2.)
    {
      return interfaceValue(y - .5 - _width/2.);
    }
    else if (y > .5 - _width/2.)
    {
      return cmax;
    }
    else if (y > .5 - _width/2. - _buffer - _interface)
    {
      return interfaceValue(.5 - _width/2. - y);
    }
    else
    {
      return cmin;
    }
  }
  else if (x > (.5 - _length/2. - _buffer - _interface))
  {
    if (y > .5 + _width/2. + _buffer + _interface)
    {
      return cmin;
    }
    else if (y > .5 + _width/2.)
    {
      Real xd = x - (.5 - _length/2.);
      Real yd = y - .5 - _width/2.;
      Real r = std::sqrt(xd*xd+yd*yd);
      return interfaceValue(r);
    }
    else if (y > .5 - _width/2.)
    {
      return interfaceValue(.5 - _length/2. - x);
    }
    else if (y > .5 - _width/2. - _buffer - _interface)
    {
      Real xd = x - (.5 - _length/2.);
      Real yd = y - .5 + _width/2.;
      Real r = std::sqrt(xd*xd+yd*yd);
      return interfaceValue(r);
    }
    else
    {
      return cmin;
    }
  }
  else
  {
    return cmin;
  }
}

RealGradient CrossIC::gradient(const Point & p)
{
  Point pxminus = p, pxplus = p, pyminus = p, pyplus = p;
  pxminus(0) -= TOLERANCE;
  pyminus(1) -= TOLERANCE;
  pxplus(0)  += TOLERANCE;
  pyplus(1)  += TOLERANCE;
  Number uxminus = value(pxminus),
    uxplus  = value(pxplus),
    uyminus = value(pyminus),
    uyplus  = value(pyplus);
  return Gradient((uxplus-uxminus)/2./TOLERANCE,
                  (uyplus-uyminus)/2./TOLERANCE);
}
