/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TRICRYSTAL2CIRCLEGRAINSIC_H
#define TRICRYSTAL2CIRCLEGRAINSIC_H

#include "Kernel.h"
#include "InitialCondition.h"

// System includes
#include <string>

// Forward Declarations
class Tricrystal2CircleGrainsIC;

template<>
InputParameters validParams<Tricrystal2CircleGrainsIC>();

/**
 * Tricrystal2CircleGrainsIC creates a 3 grain structure with 2 circle grains and one matrix grain
*/
class Tricrystal2CircleGrainsIC : public InitialCondition
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  Tricrystal2CircleGrainsIC(const InputParameters & parameters);

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */
  virtual Real value(const Point & p);

protected:
  MooseMesh & _mesh;

  unsigned int _op_num;
  unsigned int _op_index;

  Point _bottom_left;
  Point _top_right;
  Point _range;
};

#endif //TRICRYSTAL2CIRCLEGRAINSIC_H
