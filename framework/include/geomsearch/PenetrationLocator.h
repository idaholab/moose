/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef PENETRATIONLOCATOR_H
#define PENETRATIONLOCATOR_H

// Moose includes
#include "GeometricSearchInterface.h"
#include "Restartable.h"
#include "PenetrationInfo.h"

// libmesh includes
#include "libmesh/libmesh_common.h"
#include "MooseMesh.h"
#include "libmesh/vector_value.h"
#include "libmesh/point.h"
#include "libmesh/fe_type.h"
#include "libmesh/fe.h"

#include <vector>
#include <map>

//Forward Declarations
class SubProblem;
class MooseMesh;
class GeometricSearchData;

class PenetrationLocator : Restartable
{
public:
  PenetrationLocator(SubProblem & subproblem, GeometricSearchData & geom_search_data, MooseMesh & mesh, const unsigned int master_id, const unsigned int slave_id, Order order, NearestNodeLocator & nearest_node);
  ~PenetrationLocator();
  void detectPenetration();

  /**
   * Completely redo the search from scratch.
   * This is probably getting called because of mesh adaptivity.
   */
  void reinit();

  Real penetrationDistance(unsigned int node_id);
  RealVectorValue penetrationNormal(unsigned int node_id);

  enum NORMAL_SMOOTHING_METHOD
  {
    NSM_EDGE_BASED,
    NSM_NODAL_NORMAL_BASED
  };

  SubProblem & _subproblem;

  Real normDistance(const Elem & elem, const Elem & side, const Node & p0, Point & closest_point, RealVectorValue & normal);

  int intersect2D_Segments( Point S1P0, Point S1P1, Point S2P0, Point S2P1, Point* I0, Point* I1 );
  int inSegment(Point P, Point SP0, Point SP1);

  MooseMesh & _mesh;
  BoundaryID _master_boundary;
  BoundaryID _slave_boundary;

  FEType _fe_type;

  // One FE for each thread
  std::vector<FEBase * > _fe;

  NearestNodeLocator & _nearest_node;

  /// Data structure of nodes and their associated penetration information
  std::map<unsigned int, PenetrationInfo *> _penetration_info;

  std::set<unsigned int> _has_penetrated;
  std::map<unsigned int, unsigned> _locked_this_step;
  std::map<unsigned int, unsigned> _unlocked_this_step;
  std::map<unsigned int, Real> _lagrange_multiplier;

  void setUpdate(bool update);
  void setTangentialTolerance(Real tangential_tolerance);
  void setNormalSmoothingDistance(Real normal_smoothing_distance);
  void setNormalSmoothingMethod(std::string nsmString);
  void saveContactStateVars();

protected:
  bool _update_location; // Update the penetration location for nodes found last time
  Real _tangential_tolerance; // Tangential distance a node can be from a face and still be in contact
  bool _do_normal_smoothing;  // Should we do contact normal smoothing?
  Real _normal_smoothing_distance; // Distance from edge (in parametric coords) within which to perform normal smoothing
  NORMAL_SMOOTHING_METHOD _normal_smoothing_method;
};

#endif //PENETRATIONLOCATOR_H
