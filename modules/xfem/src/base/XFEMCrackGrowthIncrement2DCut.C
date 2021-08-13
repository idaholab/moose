//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMCrackGrowthIncrement2DCut.h"
#include "XFEMFuncs.h"
#include "GeometricCutUserObject.h"
#include "MooseError.h"
#include "libmesh/string_to_enum.h"

XFEMCrackGrowthIncrement2DCut::XFEMCrackGrowthIncrement2DCut(
    Real x0, Real y0, Real x1, Real y1, Real t0, Real t1)
  : _time_range(std::make_pair(t0, t1)),
    _cut_line_endpoints(std::make_pair(Point(x0, y0, 0.0), Point(x1, y1, 0.0)))
{
}

Real
XFEMCrackGrowthIncrement2DCut::cutCompletionFraction(Real time)
{
  Real fraction = 0.0;
  if (time >= _time_range.first)
  {
    if (time >= _time_range.second)
      fraction = 1.0;
    else
      fraction = (time - _time_range.first) / (_time_range.second - _time_range.first);
  }
  return fraction;
}

bool
XFEMCrackGrowthIncrement2DCut::cutElementByCrackGrowthIncrement(
    const Elem * elem, std::vector<CutEdgeForCrackGrowthIncr> & cut_edges, Real time)
{
  bool cut_elem = false;

  Real fraction = cutCompletionFraction(time);

  if (fraction > 0.0)
  {
    unsigned int n_sides = elem->n_sides();

    for (unsigned int i = 0; i < n_sides; ++i)
    {
      // This returns the lowest-order type of side, which should always
      // be an EDGE2 here because this class is for 2D only.
      std::unique_ptr<const Elem> curr_side = elem->side_ptr(i);
      if (curr_side->type() != EDGE2)
        mooseError("In cutElementByGeometry element side must be EDGE2, but type is: ",
                   libMesh::Utility::enum_to_string(curr_side->type()),
                   " base element type is: ",
                   libMesh::Utility::enum_to_string(elem->type()));

      const Node * node1 = curr_side->node_ptr(0);
      const Node * node2 = curr_side->node_ptr(1);
      Real seg_int_frac = 0.0;

      if (Xfem::intersectSegmentWithCutLine(
              *node1, *node2, _cut_line_endpoints, fraction, seg_int_frac))
      {
        cut_elem = true;
        CutEdgeForCrackGrowthIncr mycut;
        mycut._id1 = node1->id();
        mycut._id2 = node2->id();
        mycut._distance = seg_int_frac;
        mycut._host_side_id = i;
        cut_edges.push_back(mycut);
      }
    }
  }
  return cut_elem;
}
