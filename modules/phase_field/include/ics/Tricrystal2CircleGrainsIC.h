//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TRICRYSTAL2CIRCLEGRAINSIC_H
#define TRICRYSTAL2CIRCLEGRAINSIC_H

#include "InitialCondition.h"

// Forward Declarations
class Tricrystal2CircleGrainsIC;

template <>
InputParameters validParams<Tricrystal2CircleGrainsIC>();

/**
 * Tricrystal2CircleGrainsIC creates a 3 grain structure with 2 circle grains and one matrix grain
*/
class Tricrystal2CircleGrainsIC : public InitialCondition
{
public:
  Tricrystal2CircleGrainsIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

protected:
  const MooseMesh & _mesh;

  const unsigned int _op_num;
  const unsigned int _op_index;

  Point _bottom_left;
  Point _top_right;
  Point _range;
};

#endif // TRICRYSTAL2CIRCLEGRAINSIC_H
