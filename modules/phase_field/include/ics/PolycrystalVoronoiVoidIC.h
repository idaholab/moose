/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALVORONOIVOIDIC_H
#define POLYCRYSTALVORONOIVOIDIC_H

#include "MultiSmoothCircleIC.h"
#include "MooseRandom.h"
#include "PolycrystalICTools.h"

// Forward Declarationsc
class PolycrystalVoronoiVoidIC;

template <>
InputParameters validParams<PolycrystalVoronoiVoidIC>();

/**
 * PolycrystalVoronoiVoidIC initializes either grain or void values for a
 * voronoi tesselation with voids distributed along the grain boundaries.
 */
class PolycrystalVoronoiVoidIC : public MultiSmoothCircleIC
{
public:
  PolycrystalVoronoiVoidIC(const InputParameters & parameters);

  virtual void initialSetup() override;

  static InputParameters actionParameters();

protected:
  const MooseEnum _structure_type;

  const unsigned int _op_num;
  const unsigned int _grain_num;
  const unsigned int _op_index;

  const unsigned int _rand_seed;

  const bool _columnar_3D;

  virtual void computeCircleCenters() override;

  virtual Real value(const Point & p) override;
  virtual RealGradient gradient(const Point & p) override;

  virtual Real grainValueCalc(const Point & p);
  virtual void computeGrainCenters();

  std::vector<Point> _centerpoints;
  std::vector<unsigned int> _assigned_op;

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

#endif // POLYCRYSTALVORONOIVOIDIC_H
