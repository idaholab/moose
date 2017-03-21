/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef THUMBIC_H
#define THUMBIC_H

#include "InitialCondition.h"

// Forward Declarations
class ThumbIC;

template <>
InputParameters validParams<ThumbIC>();

/**
 * ThumbIC creates a rectangle with a half circle on top
 */
class ThumbIC : public InitialCondition
{
public:
  ThumbIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

protected:
  const Real _xcoord;
  const Real _width;
  const Real _height;
  const Real _invalue;
  const Real _outvalue;
};

#endif // THUMBIC_H
