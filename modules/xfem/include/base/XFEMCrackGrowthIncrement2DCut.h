//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
