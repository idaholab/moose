/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef THUMBIC_H
#define THUMBIC_H

#include "Kernel.h"
#include "InitialCondition.h"

// System includes
#include <string>

// Forward Declarations
class ThumbIC;

template<>
InputParameters validParams<ThumbIC>();

/**
 * ThumbIC creates a rectangle with a half circle on top
 */
class ThumbIC : public InitialCondition
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  ThumbIC(const InputParameters & parameters);

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */
  virtual Real value(const Point & p);

protected:
  Real _xcoord;
  Real _width;
  Real _height;
  Real _invalue;
  Real _outvalue;
};

#endif //THUMBIC_H
