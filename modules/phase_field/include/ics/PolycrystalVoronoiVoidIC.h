#ifndef POLYCRYSTALVORONOIVOIDIC_H
#define POLYCRYSTALVORONOIVOIDIC_H

#include "MultiSmoothCircleIC.h"
#include "MooseRandom.h"
#include "PolycrystalICTools.h"

// Forward Declarationsc
class PolycrystalVoronoiVoidIC;

template<>
InputParameters validParams<PolycrystalVoronoiVoidIC>();

/**
 * PolycrystalVoronoiVoidIC initializes either grain or void values for a voronoi tesselation with voids distributed along the grain boundaries.
 */
class PolycrystalVoronoiVoidIC :
  public MultiSmoothCircleIC
{
public:
  PolycrystalVoronoiVoidIC(const InputParameters & parameters);

private:

  MooseEnum _structure_type;

  unsigned int _op_num;
  unsigned int _grain_num;
  unsigned int _op_index;

  unsigned int _rand_seed;

  bool _columnar_3D;

  virtual void initialSetup();

  virtual void computeCircleCenters();

  virtual void computeGrainCenters();

  virtual Real value(const Point & p);

  virtual Real grain_value_calc(const Point & p);

  virtual RealGradient gradient(const Point & p);

  std::vector<Point> _centerpoints;
  std::vector<Real> _assigned_op;

  // define type for distance and point
  struct DistancePoint
  {
    Real d;
    unsigned int gr;
  };

  // Sort the temp_centerpoints into order of magnitude
  struct DistancePointComparator
  {
    bool operator () (const DistancePoint & a, const DistancePoint & b)
      {
        return a.d < b.d;
      }
  } _customLess;
};

#endif //POLYCRYSTALVORONOIVOIDIC_H
