//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "MooseMesh.h"
#include "libmesh/mesh.h"
#include "libmesh/elem.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/elem_side_builder.h"
#include "libmesh/parallel_object.h"
#include "MooseMesh.h"
class RayTracingOverlayMeshMapping : public GeneralUserObject
{
public:
  RayTracingOverlayMeshMapping(const InputParameters & parameters);

  static InputParameters validParams();

  void initialize();
  void finalize(){};
  void execute(){};

  // get the name of the overlay mesh
  const MeshGeneratorName & getOverlayMeshName() const { return _overlay_mesh_name; };
  // get the name of the main mesh
  const MeshGeneratorName & getMainMeshName() const { return _main_mesh_name; };

  /**
   * Store the intersection elems in two-way maps
   * overlay -> main map use overlay elem as key
   * main -> overlay map use main elem at the boundary as key
   */
  void ElemIntersectionMap();

  /**
   * Determines the intersection points between the elements \p elem and \p cut_elem
   * and fills them into \p intersection_points.
   *
   * credit to @loganharbour
   */
  bool isElemIntersection(const Elem * elem, const Elem * cut_elem);

  /**
   * Whether or not the edges \p edge1 and \p edge2 intersect.
   * If they do intersect, the intersection is filled into \p intersection_point.
   *
   * credit to @loganharbour
   */
  bool edgesIntersect(const Elem & edge1, const Elem & edge2, Point & intersection_point) const;

  /**
   * Whether or not \p edge and \p side intersect.
   * If they do intersect, the intersection is filled into \p intersection_point.
   *
   * credit to @loganharbour
   */
  bool edgeIntersectsSide(const Elem & edge, const Elem & side, Point & intersection_point) const;

  /**
   * Sync all the local mappings to rank 0
   */
  void sync_map(std::map<dof_id_type, std::set<dof_id_type>> & overlay_map,
                std::map<dof_id_type, std::set<dof_id_type>> & main_overlay_map) const;

  /**
   * Return the intersection elem IDs in two-way mapping
   * @param to_overlay is set to true to get mapping from main mesh elem to overlay mesh elems,
   * false vice versa
   * @param serialize is set to true to serilize the data to rank 0, false to have a distributed
   * mapping
   * @return an map of intersection elem ID between main and overlay mesh
   */
  std::map<dof_id_type, std::set<dof_id_type>> overlayIDMap(const bool to_overlay,
                                                            const bool serialize) const;

  /// Helper for building element sides without extraneous allocation
  ElemSideBuilder _elem_side_builder;

private:
  // name of the main mesh
  const MeshGeneratorName _main_mesh_name;
  // name of the overlay mesh
  const MeshGeneratorName _overlay_mesh_name;
  // main mesh
  libMesh::MeshBase & _main_mesh;
  // overlay mesh
  const std::unique_ptr<libMesh::MeshBase> _overlay_mesh;
  // Contain intersection elem IDs between overlay mesh and main mesh
  std::map<dof_id_type, std::set<dof_id_type>> _main_overlay_map;
  // Contain intersection elems in overlay mesh
  std::map<const Elem *, std::vector<const Elem *>> _from_overlay;
  // Contain intersection elems in main mesh
  std::map<const Elem *, std::vector<const Elem *>> _to_overlay;
};
