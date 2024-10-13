//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "MooseTypes.h"
#include "Restartable.h"
#include "PenetrationInfo.h"
#include "PerfGraphInterface.h"

#include "libmesh/vector_value.h"
#include "libmesh/point.h"
#include "libmesh/fe_base.h"

// Forward Declarations
class SubProblem;
class MooseMesh;
class GeometricSearchData;
class NearestNodeLocator;

class PenetrationLocator : Restartable, public PerfGraphInterface
{
public:
  PenetrationLocator(SubProblem & subproblem,
                     GeometricSearchData & geom_search_data,
                     MooseMesh & mesh,
                     const unsigned int primary_id,
                     const unsigned int secondary_id,
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
  BoundaryID _primary_boundary;
  BoundaryID _secondary_boundary;

  libMesh::FEType _fe_type;

  // One FE for each thread and for each dimension
  std::vector<std::vector<libMesh::FEBase *>> _fe;

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

  const Moose::PatchUpdateType _patch_update_strategy; // Contact patch update strategy
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
