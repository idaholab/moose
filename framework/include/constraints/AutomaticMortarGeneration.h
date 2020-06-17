//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarSegmentInfo.h"
#include "MooseHashing.h"
#include "ConsoleStreamInterface.h"

// libMesh includes
#include "libmesh/id_types.h"
#include "libmesh/equation_systems.h"
#include "libmesh/elem.h"
#include "libmesh/replicated_mesh.h"

// C++ includes
#include <set>
#include <memory>
#include <vector>
#include <unordered_map>

// Forward declarations
namespace libMesh
{
class MeshBase;
class Elem;
}
class GetPot;

// Using statements
using libMesh::boundary_id_type;
using libMesh::dof_id_type;
using libMesh::Elem;
using libMesh::MeshBase;
using libMesh::Node;
using libMesh::Point;
using libMesh::Real;
using libMesh::ReplicatedMesh;
using libMesh::subdomain_id_type;

typedef boundary_id_type BoundaryID;
typedef subdomain_id_type SubdomainID;

/**
 * This class is a container/interface for the objects involved in
 * automatic generation of mortar spaces.
 */
class AutomaticMortarGeneration : public ConsoleStreamInterface
{
public:
  /**
   * The name of the nodal normals system. We store this in one place
   * so it's easy to change later.
   */
  const static std::string system_name;

  /**
   * Must be constructed with a reference to the Mesh we are
   * generating mortar spaces for.
   */
  AutomaticMortarGeneration(MooseApp & app,
                            MeshBase & mesh_in,
                            const std::pair<BoundaryID, BoundaryID> & boundary_key,
                            const std::pair<SubdomainID, SubdomainID> & subdomain_key,
                            bool on_displaced,
                            bool periodic);

  /**
   * Once the secondary_requested_boundary_ids and
   * master_requested_boundary_ids containers have been filled in,
   * call this function to build node-to-Elem maps for the
   * lower-dimensional elements.
   */
  void buildNodeToElemMaps();

  /**
   * Computes and stores the nodal normal vectors in a local data
   * structure instead of using the ExplicitSystem/NumericVector
   * approach. This design was triggered by the way that the
   * GhostingFunctor operates, but I think it is a better/more
   * efficient way to do it anyway.
   */
  void computeNodalNormals();

  /**
   * Since the nodal normals are no longer a variable in the
   * EquationSystems, we need to have an alternate method for writing
   * them out to file for visualization.
   */
  void writeNodalNormalsToFile();

  /**
   * Project secondary nodes (find xi^(2) values) to the closest points on
   * the master surface.
   * Inputs:
   * - The nodal normals values
   * - mesh
   * - nodes_to_master_elem_map
   *
   * Outputs:
   * - secondary_node_and_elem_to_xi2_master_elem
   *
   * Defined in the file project_secondary_nodes.C.
   */
  void projectSlaveNodes();

  /**
   * (Inverse) project master nodes to the points on the secondary surface
   * where they would have come from (find (xi^(1) values)).
   *
   * Inputs:
   * - The nodal normals values
   * - mesh
   * - nodes_to_secondary_elem_map
   *
   * Outputs:
   * - master_node_and_elem_to_xi1_secondary_elem
   *
   * Defined in the file project_master_nodes.C.
   */
  void projectMasterNodes();

  /**
   * Builds the mortar segment mesh once the secondary and master node
   * projections have been completed.
   *
   * Inputs:
   * - mesh
   * - master_node_and_elem_to_xi1_secondary_elem
   * - secondary_node_and_elem_to_xi2_master_elem
   * - nodes_to_master_elem_map
   *
   * Outputs:
   * - mortar_segment_mesh
   * - msm_elem_to_info
   *
   * Defined in the file build_mortar_segment_mesh.C.
   */
  void buildMortarSegmentMesh();

  /**
   * Clears the mortar segment mesh and accompanying data structures
   */
  void clear();

  /**
   * returns whether this object is on the displaced mesh
   */
  bool onDisplaced() const { return _on_displaced; }

public:
  // Reference to the mesh stored in equation_systems.
  MeshBase & mesh;

  // The boundary ids corresponding to all the secondary surfaces.
  std::set<boundary_id_type> secondary_requested_boundary_ids;

  // The boundary ids corresponding to all the master surfaces.
  std::set<boundary_id_type> master_requested_boundary_ids;

  // A list of master/secondary boundary id pairs corresponding to each
  // side of the mortar interface.
  std::vector<std::pair<boundary_id_type, boundary_id_type>> master_secondary_boundary_id_pairs;

  // Map from nodes to connected lower-dimensional elements on the secondary/master subdomains.
  std::unordered_map<dof_id_type, std::vector<const Elem *>> nodes_to_secondary_elem_map;
  std::unordered_map<dof_id_type, std::vector<const Elem *>> nodes_to_master_elem_map;

  // Similar to the map above, but associates a (Slave Node, Slave Elem)
  // pair to a (xi^(2), master Elem) pair. This allows a single secondary node, which is
  // potentially connected to two elements on the secondary side, to be associated with
  // multiple master Elem/xi^(2) values to handle the case where the master and secondary
  // nodes are "matching".
  // In this configuration:
  //
  //    A     B
  // o-----o-----o  (secondary orientation ->)
  //       |
  //       v
  // ------x------ (master orientation <-)
  //    C     D
  //
  // The entries in the map should be:
  // (Elem A, Node 1) -> (Elem C, xi^(2)=-1)
  // (Elem B, Node 0) -> (Elem D, xi^(2)=+1)
  std::unordered_map<std::pair<const Node *, const Elem *>, std::pair<Real, const Elem *>>
      secondary_node_and_elem_to_xi2_master_elem;

  // Same type of container, but for mapping (Master Node ID, Master Node,
  // Master Elem) -> (xi^(1), Slave Elem) where they are inverse-projected along
  // the nodal normal direction. Note that the first item of the key, the master
  // node ID, is important for storing the key-value pairs in a consistent order
  // across processes, e.g. this container has to be ordered!
  std::map<std::tuple<dof_id_type, const Node *, const Elem *>, std::pair<Real, const Elem *>>
      master_node_and_elem_to_xi1_secondary_elem;

  // 1D Mesh of mortar segment elements which gets built by the call
  // to build_mortar_segment_mesh().
  ReplicatedMesh mortar_segment_mesh;

  // Map between Elems in the mortar segment mesh and their info
  // structs. This gets filled in by the call to
  // build_mortar_segment_mesh().
  std::unordered_map<const Elem *, MortarSegmentInfo> msm_elem_to_info;

  // Keeps track of the mapping between lower-dimensional elements and
  // the side_id of the interior_parent which they are.
  std::unordered_map<const Elem *, unsigned int> lower_elem_to_side_id;

  // The amount by which elements in the boundary subdomains
  // are offset from their respective interior parent's subdomain ids.
  const subdomain_id_type boundary_subdomain_id_offset = 1000;

  // A list of master/secondary subdomain id pairs corresponding to each
  // side of the mortar interface.
  std::vector<std::pair<subdomain_id_type, subdomain_id_type>> master_secondary_subdomain_id_pairs;

  // The secondary/master lower-dimensional boundary subdomain ids are the
  // secondary/master *boundary* ids offset by the value above.
  std::set<subdomain_id_type> secondary_boundary_subdomain_ids;
  std::set<subdomain_id_type> master_boundary_subdomain_ids;

  // Size of the largest secondary side lower-dimensional element added to
  // the mesh. Used in plotting convergence data.
  Real h_max;

  // Used by the AugmentSparsityOnInterface functor to determine
  // whether a given Elem is coupled to any others across the gap, and
  // to explicitly set up the dependence between interior_parent()
  // elements on the secondary side and their lower-dimensional sides
  // which are on the interface. This latter type of coupling must be
  // explicitly declared when there is no master_elem for a given
  // mortar segment and you are using e.g.  a P^1-P^0 discretization
  // which does not induce the coupling automatically.
  std::unordered_multimap<dof_id_type, dof_id_type> mortar_interface_coupling;

  // Container for storing the nodal normal vector associated with each secondary node.
  std::unordered_map<const Node *, Point> secondary_node_to_nodal_normal;

private:
  /**
   * Helper function responsible for projecting secondary nodes
   * onto master elements for a single master/secondary pair. Called by the class member
   * AutomaticMortarGeneration::project_secondary_nodes().
   */
  void projectSlaveNodesSinglePair(subdomain_id_type lower_dimensional_master_subdomain_id,
                                   subdomain_id_type lower_dimensional_secondary_subdomain_id);

  /**
   * Helper function used internally by AutomaticMortarGeneration::project_master_nodes().
   */
  void projectMasterNodesSinglePair(subdomain_id_type lower_dimensional_master_subdomain_id,
                                    subdomain_id_type lower_dimensional_secondary_subdomain_id);

private:
  /// Whether to print debug output
  bool _debug;

  /// Whether this object is on the displaced mesh
  const bool _on_displaced;

  /// Whether this object will be generating a mortar segment mesh for periodic constraints
  const bool _periodic;
};
