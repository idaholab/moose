/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MULTIBOUNDINGBOXIC_H
#define MULTIBOUNDINGBOXIC_H

#include "InitialCondition.h"

// Forward Declarations
class MultiBoundingBoxIC;

template <>
InputParameters validParams<MultiBoundingBoxIC>();

/**
 * MultiBoundingBoxIC allows setting the initial condition of a value inside and outside of a
 * specified box.
 * The box is aligned with the x,y,z axis... and is specified by passing in the x,y,z coordinates of
 * opposite corners.
 */
class MultiBoundingBoxIC : public InitialCondition
{
public:
  MultiBoundingBoxIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

protected:
  ///@{ lists of opposite corners
  std::vector<Point> _c1;
  std::vector<Point> _c2;
  ///@}

  /// number of boxes
  const unsigned int _nbox;

  /// dimensionality of the mesh
  const unsigned int _dim;

  /// values inside the boxes
  std::vector<Real> _inside;

  /// values outside the boxes
  const Real _outside;
};

#endif // MULTIBOUNDINGBOXIC_H
