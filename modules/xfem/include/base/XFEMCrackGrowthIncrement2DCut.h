/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMCRACKGROWTHINCREMENT2DCUT_H
#define XFEMCRACKGROWTHINCREMENT2DCUT_H

#include "libmesh/libmesh_common.h"
#include "libmesh/libmesh.h" // libMesh::invalid_uint
#include "libmesh/elem.h"

using namespace libMesh;

struct CutEdgeForCrackGrowthIncr
{
  unsigned int id1;
  unsigned int id2;
  Real distance;
  unsigned int host_side_id;
};

class XFEMCrackGrowthIncrement2DCut
{
public:
  XFEMCrackGrowthIncrement2DCut(Real x0, Real y0, Real x1, Real y1, Real t0, Real t1);

  virtual bool cutElementByCrackGrowthIncrement(const Elem * elem,
                                                std::vector<CutEdgeForCrackGrowthIncr> & cut_edges,
                                                Real time);

  Real cutCompletionFraction(Real time);

protected:
  bool IntersectSegmentWithCutLine(const Point & segment_point1,
                                   const Point & segment_point2,
                                   const std::pair<Point, Point> & cutting_line_points,
                                   const Real & cutting_line_fraction,
                                   Real & segment_intersection_fraction);

  Real crossProduct2D(const Point & point_a, const Point & point_b);

  const std::pair<Real, Real> _time_range;

private:
  const std::pair<Point, Point> _cut_line_endpoints;
};

#endif // XFEMCRACKGROWTHINCREMENT2DCUT_H
