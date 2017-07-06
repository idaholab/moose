/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEM_CRACK_GROWTH_INCREMENT_2D_CUT_H
#define XFEM_CRACK_GROWTH_INCREMENT_2D_CUT_H

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
  ~XFEMCrackGrowthIncrement2DCut();

  virtual bool cutElementByCrackGrowthIncrement(const Elem * elem,
                                                std::vector<CutEdgeForCrackGrowthIncr> & cut_edges,
                                                Real time);

  Real cutCompletionFraction(Real time);

protected:
  bool IntersectSegmentWithCutLine(const Point & segment_point1,
                                   const Point & segment_point2,
                                   const Point & cutting_line_point1,
                                   const Point & cutting_line_point2,
                                   const Real & cutting_line_fraction,
                                   Real & segment_intersection_fraction);

  Real crossProduct2D(Real ax, Real ay, Real bx, Real by);

  Real _t_start;
  Real _t_end;

private:
  const Point _cut_line_start;
  const Point _cut_line_end;
};

#endif // XFEM_CRACK_GROWTH_INCREMENT_2D_CUT_H
