//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/mesh.h"
#include "libmesh/elem.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/elem_side_builder.h"
#include "MooseMesh.h"

/**
 * Helper class that computes the intersection of a line segment defined by a point and a direction
 * and a bounding box.
 */
class IntersectionElemsHelper
{
public:
  /**
   * Constructor.
   * \param dim The dimension of the bounding box
   */
  IntersectionElemsHelper(MeshBase & mesh, MeshBase & overlay_mesh);

  /**
   * Determines the intersection points between the elements \p elem and \p cut_elem
   * and fills them into \p intersection_points.
   */
  bool isElemIntersection(const Elem * elem, const Elem * cut_elem);

  void ElemIntersectionMap();

  /**
   * Whether or not the edges \p edge1 and \p edge2 intersect.
   * If they do intersect, the intersection is filled into \p intersection_point.
   */
  bool edgesIntersect(const Elem & edge1, const Elem & edge2, Point & intersection_point) const;

  /**
   * Whether or not \p edge and \p side intersect.
   * If they do intersect, the intersection is filled into \p intersection_point.
   */
  bool edgeIntersectsSide(const Elem & edge, const Elem & side, Point & intersection_point) const;

  /**
   * Get intersection elems in overlay mesh mapping to main mesh
   */
  const std::map<const Elem *, std::vector<const Elem *>> getOverlayElemstoMain()
  {
    return _overlay_elems_to_main;
  };

  /**
   * Get intersection elems in main mesh mapping to overlay mesh
   */
  const std::map<const Elem *, std::vector<const Elem *>> getMainElemstoOverlay()
  {
    return _main_elems_to_overlay;
  };

  /// Helper for building element sides without extraneous allocation
  ElemSideBuilder _elem_side_builder;

private:
  /// Dummy communicator for the dummy mesh
  libMesh::Parallel::Communicator _comm;
  /// Dummy mesh that contains a single element used for TraceRayTools intersection methods
  const MeshBase & _mesh;
  /// Dummy mesh that contains a single element used for TraceRayTools intersection methods
  const MeshBase & _overlay_mesh;
  //   /// Contain intersection elems in overlay mesh
  std::map<const Elem *, std::vector<const Elem *>> _overlay_elems_to_main;
  //   /// Contain intersection elems in main mesh
  std::map<const Elem *, std::vector<const Elem *>> _main_elems_to_overlay;
};
