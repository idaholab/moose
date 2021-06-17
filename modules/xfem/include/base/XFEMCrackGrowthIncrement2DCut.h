//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_common.h"
#include "libmesh/libmesh.h" // libMesh::invalid_uint
#include "libmesh/elem.h"

using namespace libMesh;

struct CutEdgeForCrackGrowthIncr
{
  unsigned int _id1;
  unsigned int _id2;
  Real _distance;
  unsigned int _host_side_id;
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
  const std::pair<Real, Real> _time_range;

private:
  const std::pair<Point, Point> _cut_line_endpoints;
};
