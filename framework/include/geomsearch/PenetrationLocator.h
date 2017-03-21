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
#include "Restartable.h"
#include "PenetrationInfo.h"

// libmesh includes
#include "libmesh/vector_value.h"
#include "libmesh/point.h"
#include "libmesh/fe.h"

// Forward Declarations
class SubProblem;
class MooseMesh;
class GeometricSearchData;
class NearestNodeLocator;

class PenetrationLocator : Restartable
{
public:
  PenetrationLocator(SubProblem & subproblem,
                     GeometricSearchData & geom_search_data,
                     MooseMesh & mesh,
                     const unsigned int master_id,
                     const unsigned int slave_id,
                     Order order,
                     NearestNodeLocator & nearest_node);
  ~PenetrationLocator();
  void detectPenetration();

  /**
   * Completely redo the search from scratch.
   * This is probably getting called because of mesh adaptivity.
   */
  void reinit();

  Real penetrationDistance(dof_id_type node_id);
  RealVectorValue penetrationNormal(dof_id_type node_id);

  enum NORMAL_SMOOTHING_METHOD
  {
    NSM_EDGE_BASED,
    NSM_NODAL_NORMAL_BASED
  };

  SubProblem & _subproblem;

  Real normDistance(const Elem & elem,
                    const Elem & side,
                    const Node & p0,
                    Point & closest_point,
                    RealVectorValue & normal);

  int intersect2D_Segments(Point S1P0, Point S1P1, Point S2P0, Point S2P1, Point * I0, Point * I1);
  int inSegment(Point P, Point SP0, Point SP1);

  MooseMesh & _mesh;
  BoundaryID _master_boundary;
  BoundaryID _slave_boundary;

  FEType _fe_type;

  // One FE for each thread and for each dimension
  std::vector<std::vector<FEBase *>> _fe;

  NearestNodeLocator & _nearest_node;

  /// Data structure of nodes and their associated penetration information
  std::map<dof_id_type, PenetrationInfo *> & _penetration_info;

  std::set<dof_id_type> &
      _has_penetrated; // This is only hanging around for legacy code. Don't use it!

  void setCheckWhetherReasonable(bool state);
  void setUpdate(bool update);
  void setTangentialTolerance(Real tangential_tolerance);
  void setNormalSmoothingDistance(Real normal_smoothing_distance);
  void setNormalSmoothingMethod(std::string nsmString);
  Real getTangentialTolerance() { return _tangential_tolerance; }

protected:
  /// Check whether found candidates are reasonable
  bool _check_whether_reasonable;
  bool & _update_location;         // Update the penetration location for nodes found last time
  Real _tangential_tolerance;      // Tangential distance a node can be from a face and still be in
                                   // contact
  bool _do_normal_smoothing;       // Should we do contact normal smoothing?
  Real _normal_smoothing_distance; // Distance from edge (in parametric coords) within which to
                                   // perform normal smoothing
  NORMAL_SMOOTHING_METHOD _normal_smoothing_method;
};

/**
 * We have to have a specialization for this map because the PenetrationInfo
 * objects MUST get deleted before the ones are loaded from a file or it's a memory leak.
 */
template <>
inline void
dataLoad(std::istream & stream, std::map<dof_id_type, PenetrationInfo *> & m, void * context)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator it = m.begin();
  std::map<dof_id_type, PenetrationInfo *>::iterator end = m.end();

  for (; it != end; ++it)
    delete it->second;

  m.clear();

  // First read the size of the map
  unsigned int size = 0;
  stream.read((char *)&size, sizeof(size));

  for (unsigned int i = 0; i < size; i++)
  {
    dof_id_type key;
    loadHelper(stream, key, context);
    loadHelper(stream, m[key], context);
  }
}

#endif // PENETRATIONLOCATOR_H
