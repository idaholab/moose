//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiSmoothCircleIC.h"
#include "MooseRandom.h"
#include "PolycrystalICTools.h"

// Forward Declarationsc
class GrainTrackerInterface;
class PolycrystalVoronoi;

/**
 * PolycrystalVoronoiVoidIC initializes either grain or void values for a
 * voronoi tesselation with voids distributed along the grain boundaries.
 */
class PolycrystalVoronoiVoidIC : public MultiSmoothCircleIC
{
public:
  static InputParameters validParams();

  PolycrystalVoronoiVoidIC(const InputParameters & parameters);

  virtual void initialSetup() override;

  static InputParameters actionParameters();

protected:
  const MooseEnum _structure_type;

  const unsigned int _op_num;
  const unsigned int _op_index;

  const bool _columnar_3D;

  const PolycrystalVoronoi & _poly_ic_uo;

  const FileName _file_name;

  virtual void computeCircleCenters() override;

  virtual Real value(const Point & p) override;
  virtual RealGradient gradient(const Point & p) override;

  unsigned int _grain_num;
  std::vector<Point> _centerpoints;

  /// Type for distance and point
  struct DistancePoint
  {
    Real d;
    unsigned int gr;
  };

  /// Sorts the temp_centerpoints into order of magnitude
  struct DistancePointComparator
  {
    bool operator()(const DistancePoint & a, const DistancePoint & b) { return a.d < b.d; }
  } _customLess;
};
