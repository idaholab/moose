/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CROSSIC_H
#define CROSSIC_H

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

#include "InitialCondition.h"
#include "C1ICBase.h"

// Forward Declarations
class CrossIC;

template <>
InputParameters validParams<CrossIC>();

/**
 * CrossIC creates a C1 continuous initial condition that looks like a cross in
 * the middle of the domain.
 */
class CrossIC : public C1ICBase
{
public:
  CrossIC(const InputParameters & parameters);

  virtual Real value(const Point & p);
  virtual RealGradient gradient(const Point & p);

  const Real _x1;
  const Real _y1;
  const Real _x2;
  const Real _y2;
};

#endif // CROSSIC_H
