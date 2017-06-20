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
  virtual void getGrainsBasedOnPoint(const Point & point,
                                     std::vector<unsigned int> & grains) const override;
  virtual Real getVariableValue(unsigned int op_index, const Point & p) const override;

  virtual unsigned int getNumGrains() const override { return _grain_num; }

protected:
  /// The number of grains to create
  const unsigned int _grain_num;

  const bool _columnar_3D;

  const unsigned int _rand_seed;

  Point _bottom_left;
  Point _top_right;
  Point _range;

  std::vector<Point> _centerpoints;
};

#endif // POLYCRYSTALVORONOI_H
