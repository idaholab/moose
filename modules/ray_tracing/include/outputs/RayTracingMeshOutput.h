//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Local includes
#include "RayTracingCommon.h"
#include "Ray.h"

// MOOSE includes
#include "FileOutput.h"
#include "UserObjectInterface.h"

// libMesh includes
#include "libmesh/distributed_mesh.h"
#include "libmesh/equation_systems.h"
#include "libmesh/explicit_system.h"

// Forward declarations
class RayTracingStudy;
struct TraceData;

/**
 * Base class for outputting Ray data in a mesh format, where EDGE2 elems represent the individual
 * Ray segments
 */
class RayTracingMeshOutput : public FileOutput, public UserObjectInterface
{
public:
  RayTracingMeshOutput(const InputParameters & parameters);

  static InputParameters validParams();

  virtual std::string filename() override;

  void output(const ExecFlagType & type) override;

protected:
  /**
   * Output the mesh - to be overridden
   */
  virtual void outputMesh() = 0;

  /// The RayTracingStudy
  const RayTracingStudy & _study;

  /// Whether or not to output the Ray's data
  const bool _output_data;
  /// Whether or not to output the Ray's aux data
  const bool _output_aux_data;
  /// Whether or not to output the Ray's data in a nodal, linear sense
  const bool _output_data_nodal;

  /// The mesh that contains the segments
  std::unique_ptr<MeshBase> _segment_mesh;
  /// The EquationSystems
  std::unique_ptr<EquationSystems> _es;
  /// The system that stores the field data
  ExplicitSystem * _sys;

private:
  /**
   * Build the inflated neighbor bounding boxes stored in _inflated_neighbor_bboxes for the purposes
   * of identifying processors that may contain nodes that we need to decide on ownership for.
   */
  void buildBoundingBoxes();
  /**
   * Builds a map for each Ray to starting element and node ID for the EDGE2 mesh
   * that will represent said Ray.
   */
  void buildIDMap();
  /**
   * Build the mesh that contains the ray tracing segments
   */
  void buildSegmentMesh();
  /**
   * Setup the equation system that stores the segment-wise field data
   */
  void setupEquationSystem();
  /**
   * Fill the Ray field data
   */
  void fillFields();

  /**
   * Gets the number of nodes needed to represent a given trace.
   */
  dof_id_type neededNodes(const TraceData & trace_data) const;
  /**
   * Gets the starting node and element IDs in the EDGE2 mesh for a given trace.
   */
  void startingIDs(const TraceData & trace_data,
                   dof_id_type & start_node_id,
                   dof_id_type & start_elem_id) const;

  /// The variable index in _sys for the Ray's ID (if any)
  unsigned int _ray_id_var;
  /// The variable index in _sys for the intersection ID (if any)
  unsigned int _intersections_var;
  /// The variable index in _sys for the Ray's processor id (if any)
  unsigned int _pid_var;
  /// The variable index in _sys for the Ray's processor crossings (if any)
  unsigned int _processor_crossings_var;
  /// The variable index in _sys for the Ray's trajectory changes  (if any)
  unsigned int _trajectory_changes_var;
  /// The first variable index in _sys for the Ray's data (if any)
  unsigned int _data_start_var;
  /// The first variable index in _sys for the Ray's aux data  (if any)
  unsigned int _aux_data_start_var;

  /// The bounding box for this processor
  BoundingBox _bbox;
  /// The inflated bounding boxes for all processors
  std::vector<BoundingBox> _inflated_bboxes;
  /// Inflated bounding boxes that are neighboring to this processor (pid : bbox for each entry)
  std::vector<std::pair<processor_id_type, BoundingBox>> _inflated_neighbor_bboxes;

  /// The map from RayID to the starting element and node ID of the mesh element for said Ray
  std::unordered_map<RayID, std::pair<dof_id_type, dof_id_type>> _ray_starting_id_map;
  /// The max node ID for the ray tracing mesh for creating unique elem IDs
  dof_id_type _max_node_id;
};
