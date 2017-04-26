/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALVORONOI_H
#define POLYCRYSTALVORONOI_H

#include "PolycrystalUserObjectBase.h"

// Forward Declarations
class PolycrystalVoronoi;

template <>
InputParameters validParams<PolycrystalVoronoi>();

class PolycrystalVoronoi : public PolycrystalUserObjectBase
{
public:
  PolycrystalVoronoi(const InputParameters & parameters);

  virtual void precomputeGrainStructure() override;

  virtual unsigned int getGrainBasedOnPoint(const Point & point) const override;

protected:
  bool _columnar_3D;

  Point _bottom_left;
  Point _top_right;
  Point _range;

  unsigned int _rand_seed;

  std::vector<Point> _centerpoints;
};

#endif // POLYCRYSTALVORONOI_H
