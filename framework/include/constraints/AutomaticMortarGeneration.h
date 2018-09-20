#ifndef AUTOMATIC_MORTAR_GENERATION_H
#define AUTOMATIC_MORTAR_GENERATION_H

#include "MortarSegmentInfo.h"

// libMesh includes
#include "libmesh/id_types.h"
#include "libmesh/equation_systems.h"
#include "libmesh/elem.h"
#include "libmesh/mesh.h"

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
using libMesh::Elem;
using libMesh::Mesh;
using libMesh::MeshBase;
using libMesh::Node;
using libMesh::Point;
using libMesh::Real;
using libMesh::boundary_id_type;
using libMesh::dof_id_type;
using libMesh::subdomain_id_type;

typedef boundary_id_type BoundaryID;
typedef subdomain_id_type SubdomainID;

/**
 * This class is a container/interface for the objects involved in
 * automatic generation of mortar spaces.
 */
class AutomaticMortarGeneration
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
  AutomaticMortarGeneration(MeshBase & mesh_in,
                            const std::pair<BoundaryID, BoundaryID> & boundary_key,
                            const std::pair<SubdomainID, SubdomainID> & subdomain_key);

  /**
   * Once the slave_requested_boundary_ids and
   * master_requested_boundary_ids containers have been filled in,
   * call this function to build node-to-Elem maps for the
   * lower-dimensional elements.
   */
  void build_node_to_elem_maps();

  /**
   * Computes and stores the nodal normal vectors in a local data
   * structure instead of using the ExplicitSystem/NumericVector
   * approach. This design was triggered by the way that the
   * GhostingFunctor operates, but I think it is a better/more
   * efficient way to do it anyway.
   */
  void compute_nodal_normals();

  /**
   * Since the nodal normals are no longer a variable in the
   * EquationSystems, we need to have an alternate method for writing
   * them out to file for visualization.
   */
  void write_nodal_normals_to_file();

  /**
   * Project slave nodes (find xi^(2) values) to the closest points on
   * the master surface.
   * Inputs:
   * - The nodal normals values
   * - mesh
   * - nodes_to_master_elem_map
   *
   * Outputs:
   * - slave_node_and_elem_to_xi2_master_elem
   *
   * Defined in the file project_slave_nodes.C.
   */
  void project_slave_nodes();

  /**
   * (Inverse) project master nodes to the points on the slave surface
   * where they would have come from (find (xi^(1) values)).
   *
   * Inputs:
   * - The nodal normals values
   * - mesh
   * - nodes_to_slave_elem_map
   *
   * Outputs:
   * - master_node_and_elem_to_xi1_slave_elem
   *
   * Defined in the file project_master_nodes.C.
   */
  void project_master_nodes();

  /**
   * Builds the mortar segment mesh once the slave and master node
   * projections have been completed.
   *
   * Inputs:
   * - mesh
   * - master_node_and_elem_to_xi1_slave_elem
   * - slave_node_and_elem_to_xi2_master_elem
   * - nodes_to_master_elem_map
   *
   * Outputs:
   * - mortar_segment_mesh
   * - msm_elem_to_info
   *
   * Defined in the file build_mortar_segment_mesh.C.
   */
  void build_mortar_segment_mesh();

  // Reference to the mesh stored in equation_systems.
  MeshBase & mesh;

  // The boundary ids corresponding to all the slave surfaces.
  std::set<boundary_id_type> slave_requested_boundary_ids;

  // The boundary ids corresponding to all the master surfaces.
  std::set<boundary_id_type> master_requested_boundary_ids;

  // A list of master/slave boundary id pairs corresponding to each
  // side of the mortar interface.
  std::vector<std::pair<boundary_id_type, boundary_id_type>> master_slave_boundary_id_pairs;

  // Map from nodes to connected lower-dimensional elements on the slave/master subdomains.
  std::map<dof_id_type, std::vector<const Elem *>> nodes_to_slave_elem_map;
  std::map<dof_id_type, std::vector<const Elem *>> nodes_to_master_elem_map;

  // Similar to the map above, but associates a (Slave Node, Slave Elem)
  // pair to a (xi^(2), master Elem) pair. This allows a single slave node, which is
  // potentially connected to two elements on the slave side, to be associated with
  // multiple master Elem/xi^(2) values to handle the case where the master and slave
  // nodes are "matching".
  // In this configuration:
  //
  //    A     B
  // o-----o-----o  (slave orientation ->)
  //       |
  //       v
  // ------x------ (master orientation <-)
  //    C     D
  //
  // The entries in the map should be:
  // (Elem A, Node 1) -> (Elem C, xi^(2)=-1)
  // (Elem B, Node 0) -> (Elem D, xi^(2)=+1)
  std::map<std::pair<const Node *, const Elem *>, std::pair<Real, const Elem *>>
      slave_node_and_elem_to_xi2_master_elem;

  // Same type of container, but for mapping
  // (Master Node, Master Elem) -> (xi^(1), Slave Elem)
  // where they are inverse-projected along the nodal normal
  // direction.
  std::map<std::pair<const Node *, const Elem *>, std::pair<Real, const Elem *>>
      master_node_and_elem_to_xi1_slave_elem;

  // 1D Mesh of mortar segment elements which gets built by the call
  // to build_mortar_segment_mesh().
  Mesh mortar_segment_mesh;

  // Map between Elems in the mortar segment mesh and their info
  // structs. This gets filled in by the call to
  // build_mortar_segment_mesh().
  std::map<const Elem *, MortarSegmentInfo> msm_elem_to_info;

  // Keeps track of the mapping between lower-dimensional elements and
  // the side_id of the interior_parent which they are.
  std::map<const Elem *, unsigned int> lower_elem_to_side_id;

  // The amount by which elements in the boundary subdomains
  // are offset from their respective interior parent's subdomain ids.
  const subdomain_id_type boundary_subdomain_id_offset = 1000;

  // The slave/master lower-dimensional boundary subdomain ids are the
  // slave/master *boundary* ids offset by the value above.
  std::set<subdomain_id_type> slave_boundary_subdomain_ids;
  std::set<subdomain_id_type> master_boundary_subdomain_ids;

  // Size of the largest slave side lower-dimensional element added to
  // the mesh. Used in plotting convergence data.
  Real h_max;

  // Used by the AugmentSparsityOnInterface functor to determine
  // whether a given Elem is coupled to any others across the gap, and
  // to explicitly set up the dependence between interior_parent()
  // elements on the slave side and their lower-dimensional sides
  // which are on the interface. This latter type of coupling must be
  // explicitly declared when there is no master_elem for a given
  // mortar segment and you are using e.g.  a P^1-P^0 discretization
  // which does not induce the coupling automatically.
  std::unordered_multimap<const Elem *, const Elem *> mortar_interface_coupling;

  // Container for storing the nodal normal vector associated with each slave node.
  std::unordered_map<const Node *, Point> slave_node_to_nodal_normal;

private:
  /**
   * Helper function responsible for projecting slave nodes
   * onto master elements for a single master/slave pair. Called by the class member
   * AutomaticMortarGeneration::project_slave_nodes().
   */
  void project_slave_nodes_single_pair(subdomain_id_type lower_dimensional_master_subdomain_id,
                                       subdomain_id_type lower_dimensional_slave_subdomain_id);

  /**
   * Helper function used internally by AutomaticMortarGeneration::project_master_nodes().
   */
  void project_master_nodes_single_pair(subdomain_id_type lower_dimensional_master_subdomain_id,
                                        subdomain_id_type lower_dimensional_slave_subdomain_id);
};

#endif
