/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALREDUCEDIC_H
#define POLYCRYSTALREDUCEDIC_H

#include "InitialCondition.h"
#include "PolycrystalICTools.h"

// Forward Declarations
class PolycrystalReducedIC;

template <>
InputParameters validParams<PolycrystalReducedIC>();

/**
 * PolycrystalReducedIC creates a polycrystal initial condition.
 * With 2 Grains, _typ = 0 results in a circular inclusion grain and _type = 1 gives a bicrystal.
 * With more than 2 grains, _typ = 0 gives set positions for 6 grains, _type = 1 gives hexagonal
 * grains for 4 grains.
 *                          _typ = 2 Gives a random voronoi structure
 */
class PolycrystalReducedIC : public InitialCondition
{
public:
  PolycrystalReducedIC(const InputParameters & parameters);

  virtual Real value(const Point & p);
  virtual void initialSetup();

protected:
  bool assignColors(const AdjacencyGraph & adjacency_matrix,
                    std::vector<unsigned int> & colors,
                    unsigned int grain) const;
  bool isGraphValid(const AdjacencyGraph & adjacency_matrix,
                    std::vector<unsigned int> & colors,
                    unsigned int grain,
                    unsigned int color) const;

  MooseMesh & _mesh;

  /// mesh dimension
  unsigned int _dim;

  unsigned int _op_num;
  unsigned int _grain_num;
  unsigned int _op_index;

  unsigned int _rand_seed;

  bool _cody_test;
  bool _columnar_3D;

  Point _bottom_left;
  Point _top_right;
  Point _range;

  bool _advanced_op_assignment;

  std::vector<Point> _centerpoints;
  std::vector<unsigned int> _assigned_op;
};

#endif // POLYCRYSTALREDUCEDIC_H
