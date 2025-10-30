//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_KOKKOS_ENABLED
#include "KokkosMesh.h"
#endif

#include "MooseObject.h"
#include "BndNode.h"
#include "BndElement.h"
#include "Restartable.h"
#include "MooseEnum.h"
#include "PerfGraphInterface.h"
#include "MooseHashing.h"
#include "MooseApp.h"
#include "FaceInfo.h"
#include "ElemInfo.h"

#include <memory> //std::unique_ptr
#include <unordered_map>
#include <unordered_set>

// libMesh
#include "libmesh/elem_range.h"
#include "libmesh/mesh_base.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/node_range.h"
#include "libmesh/nanoflann.hpp"
#include "libmesh/vector_value.h"
#include "libmesh/point.h"
#include "libmesh/partitioner.h"

class Assembly;
class RelationshipManager;
class MooseVariableBase;
class MooseAppCoordTransform;
class MooseUnits;

// libMesh forward declarations
namespace libMesh
{
class ExodusII_IO;
class QBase;
class PeriodicBoundaries;
class Partitioner;
class GhostingFunctor;
class BoundingBox;
}
// Useful typedefs
typedef libMesh::StoredRange<std::set<Node *>::iterator, Node *> SemiLocalNodeRange;

// List of supported geometrical elements
const std::string LIST_GEOM_ELEM = "EDGE EDGE2 EDGE3 EDGE4 "
                                   "QUAD QUAD4 QUAD8 QUAD9 "
                                   "TRI TRI3 TRI6 TRI7 "
                                   "HEX HEX8 HEX20 HEX27 "
                                   "TET TET4 TET10 TET14 "
                                   "PRISM PRISM6 PRISM15 PRISM18 "
                                   "PYRAMID PYRAMID5 PYRAMID13 PYRAMID14";

/**
 * Helper object for holding qp mapping info.
 */
class QpMap
{
public:
  QpMap() : _distance(std::numeric_limits<Real>::max()) {}

  /// The qp to map from
  unsigned int _from;

  /// The qp to map to
  unsigned int _to;

  /// The distance between them
  Real _distance;
};

/**
 * MooseMesh wraps a libMesh::Mesh object and enhances its capabilities
 * by caching additional data and storing more state.
 */
class MooseMesh : public MooseObject, public Restartable, public PerfGraphInterface
{
public:
  /**
   * Typical "Moose-style" constructor and copy constructor.
   */
  static InputParameters validParams();

  MooseMesh(const InputParameters & parameters);
  MooseMesh(const MooseMesh & other_mesh);
  MooseMesh() = delete;
  MooseMesh & operator=(const MooseMesh & other_mesh) = delete;

  virtual ~MooseMesh();

  // The type of libMesh::MeshBase that will be used
  enum class ParallelType
  {
    DEFAULT,
    REPLICATED,
    DISTRIBUTED
  };

  /**
   * Clone method.  Allocates memory you are responsible to clean up.
   */
  virtual MooseMesh & clone() const;

  /**
   * A safer version of the clone() method that hands back an
   * allocated object wrapped in a smart pointer. This makes it much
   * less likely that the caller will leak the memory in question.
   */
  virtual std::unique_ptr<MooseMesh> safeClone() const = 0;

  /**
   * Determine whether to use a distributed mesh. Should be called during construction
   */
  void determineUseDistributedMesh();

  /**
   * Method to construct a libMesh::MeshBase object that is normally set and used by the MooseMesh
   * object during the "init()" phase. If the parameter \p dim is not
   * provided, then its value will be taken from the input file mesh block.
   */
  std::unique_ptr<MeshBase> buildMeshBaseObject(unsigned int dim = libMesh::invalid_uint);

  /**
   * Shortcut method to construct a unique pointer to a libMesh mesh instance. The created
   * derived-from-MeshBase object will have its \p allow_remote_element_removal flag set to whatever
   * our value is. We will also attach any geometric \p RelationshipManagers that have been
   * requested by our simulation objects to the \p MeshBase object. If the parameter \p dim is not
   * provided, then its value will be taken from the input file mesh block.
   */
  template <typename T>
  std::unique_ptr<T> buildTypedMesh(unsigned int dim = libMesh::invalid_uint);

  /**
   * Method to set the mesh_base object. If this method is NOT called prior to calling init(), a
   * MeshBase object will be automatically constructed and set.
   */
  void setMeshBase(std::unique_ptr<MeshBase> mesh_base);

  /// returns MooseMesh partitioning options so other classes can use it
  static MooseEnum partitioning();

  /// returns MooseMesh element type options
  static MooseEnum elemTypes();

  /**
   * Initialize the Mesh object.  Most of the time this will turn around
   * and call build_mesh so the child class can build the Mesh object.
   *
   * However, during Recovery this will read the CPA file...
   */
  virtual void init();

  /**
   * Must be overridden by child classes.
   *
   * This is where the Mesh object is actually created and filled in.
   */
  virtual void buildMesh() = 0;

  /**
   * Returns MeshBase::mesh_dimension(), (not
   * MeshBase::spatial_dimension()!) of the underlying libMesh mesh
   * object.
   */
  virtual unsigned int dimension() const;

  /**
   * Returns MeshBase::spatial_dimension
   */
  virtual unsigned int spatialDimension() const { return _mesh->spatial_dimension(); }

  /**
   * Returns the effective spatial dimension determined by the coordinates actually used by the
   * mesh. This means that a 1D mesh that has non-zero z or y coordinates is actually a 2D or 3D
   * mesh, respectively. Likewise a 2D mesh that has non-zero z coordinates is actually 3D mesh.
   */
  virtual unsigned int effectiveSpatialDimension() const;

  /**
   * Returns the maximum element dimension on the given blocks
   */
  unsigned int getBlocksMaxDimension(const std::vector<SubdomainName> & blocks) const;

  /**
   * Returns a vector of boundary IDs for the requested element on the
   * requested side.
   */
  std::vector<BoundaryID> getBoundaryIDs(const Elem * const elem,
                                         const unsigned short int side) const;

  /**
   * Returns a const pointer to a lower dimensional element that
   * corresponds to a side of a higher dimensional element. This
   * relationship is established through an internal_parent; if there is
   * no lowerDElem, nullptr is returned.
   */
  const Elem * getLowerDElem(const Elem *, unsigned short int) const;

  /**
   * Returns the local side ID of the interior parent aligned with the lower dimensional element.
   */
  unsigned int getHigherDSide(const Elem * elem) const;

  /**
   * Returns a const reference to a set of all user-specified
   * boundary IDs.  On a distributed mesh this will *only* include
   * boundary IDs which exist on local or ghosted elements; a copy and
   * a call to _communicator.set_union() will be necessary to get the
   * global ID set.
   */
  const std::set<BoundaryID> & getBoundaryIDs() const;

  /**
   * Calls BoundaryInfo::build_node_list()/build_side_list() and *makes separate copies* of
   * Nodes/Elems in those lists.
   *
   * Allocates memory which is cleaned up in the freeBndNodes()/freeBndElems() functions.
   */
  void buildNodeList();
  void buildBndElemList();

  /**
   * If not already created, creates a map from every node to all
   * elements to which they are connected.
   */
  const std::map<dof_id_type, std::vector<dof_id_type>> & nodeToElemMap();

  /**
   * If not already created, creates a map from every node to all
   * _active_ _semilocal_ elements to which they are connected.
   * Semilocal elements include local elements and elements that share at least
   * one node with a local element.
   * \note Extra ghosted elements are not included in this map!
   */
  const std::map<dof_id_type, std::vector<dof_id_type>> & nodeToActiveSemilocalElemMap();

  /**
   * These structs are required so that the bndNodes{Begin,End} and
   * bndElems{Begin,End} functions work...
   */
  struct bnd_node_iterator;
  struct const_bnd_node_iterator;

  struct bnd_elem_iterator;
  struct const_bnd_elem_iterator;

  /**
   * Return iterators to the beginning/end of the boundary nodes list.
   */
  virtual bnd_node_iterator bndNodesBegin();
  virtual bnd_node_iterator bndNodesEnd();

  /**
   * Return iterators to the beginning/end of the boundary elements list.
   */
  virtual bnd_elem_iterator bndElemsBegin();
  virtual bnd_elem_iterator bndElemsEnd();

  /**
   * Calls BoundaryInfo::build_node_list_from_side_list().
   */
  void buildNodeListFromSideList();

  /**
   * Calls BoundaryInfo::build_side_list(), returns a std::vector of
   * (elem-id, side-id, bc-id) tuples.
   */
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> buildSideList();

  /**
   * Calls BoundaryInfo::build_active_side_list
   * @return A container of active (element, side, id) tuples.
   */
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>>
  buildActiveSideList() const;

  /**
   * Calls BoundaryInfo::side_with_boundary_id().
   */
  unsigned int sideWithBoundaryID(const Elem * const elem, const BoundaryID boundary_id) const;

  /**
   * Calls local_nodes_begin/end() on the underlying libMesh mesh object.
   */
  MeshBase::node_iterator localNodesBegin();
  MeshBase::node_iterator localNodesEnd();
  MeshBase::const_node_iterator localNodesBegin() const;
  MeshBase::const_node_iterator localNodesEnd() const;

  /**
   * Calls active_local_nodes_begin/end() on the underlying libMesh mesh object.
   */
  MeshBase::element_iterator activeLocalElementsBegin();
  const MeshBase::element_iterator activeLocalElementsEnd();
  MeshBase::const_element_iterator activeLocalElementsBegin() const;
  const MeshBase::const_element_iterator activeLocalElementsEnd() const;

  /**
   * Calls n_nodes/elem() on the underlying libMesh mesh object.
   */
  virtual dof_id_type nNodes() const;
  virtual dof_id_type nElem() const;

  virtual dof_id_type nLocalNodes() const { return _mesh->n_local_nodes(); }
  virtual dof_id_type nActiveElem() const { return _mesh->n_active_elem(); }
  virtual dof_id_type nActiveLocalElem() const { return _mesh->n_active_local_elem(); }
  virtual SubdomainID nSubdomains() const { return _mesh->n_subdomains(); }
  virtual unsigned int nPartitions() const { return _mesh->n_partitions(); }
  virtual bool skipPartitioning() const { return _mesh->skip_partitioning(); }
  virtual bool skipNoncriticalPartitioning() const;

  /**
   * Calls max_node/elem_id() on the underlying libMesh mesh object.
   * This may be larger than n_nodes/elem() in cases where the id
   * numbering is not contiguous.
   */
  virtual dof_id_type maxNodeId() const;
  virtual dof_id_type maxElemId() const;

  /**
   * Various accessors (pointers/references) for Node "i".
   *
   * If the requested node is a remote node on a distributed mesh,
   * only the query accessors are valid to call, and they return NULL.
   */
  virtual const Node & node(const dof_id_type i) const;
  virtual Node & node(const dof_id_type i);
  virtual const Node & nodeRef(const dof_id_type i) const;
  virtual Node & nodeRef(const dof_id_type i);
  virtual const Node * nodePtr(const dof_id_type i) const;
  virtual Node * nodePtr(const dof_id_type i);
  virtual const Node * queryNodePtr(const dof_id_type i) const;
  virtual Node * queryNodePtr(const dof_id_type i);

  /**
   * Various accessors (pointers/references) for Elem "i".
   *
   * If the requested elem is a remote element on a distributed mesh,
   * only the query accessors are valid to call, and they return NULL.
   */
  virtual Elem * elem(const dof_id_type i);
  virtual const Elem * elem(const dof_id_type i) const;
  virtual Elem * elemPtr(const dof_id_type i);
  virtual const Elem * elemPtr(const dof_id_type i) const;
  virtual Elem * queryElemPtr(const dof_id_type i);
  virtual const Elem * queryElemPtr(const dof_id_type i) const;

  /**
   * Setter/getter for whether the mesh is prepared
   */
  bool prepared() const;
  virtual void prepared(bool state);

  /**
   * If this method is called, we will call libMesh's prepare_for_use method when we
   * call Moose's prepare method. This should only be set when the mesh structure is changed
   * by MeshGenerators (i.e. Element deletion).
   */
  void needsPrepareForUse();

  /**
   * Declares that the MooseMesh has changed, invalidates cached data
   * and rebuilds caches.  Sets a flag so that clients of the
   * MooseMesh also know when it has changed.
   */
  void meshChanged();

  /**
   * Declares a callback function that is executed at the conclusion
   * of meshChanged(). Ther user can implement actions required after
   * changing the mesh here.
   **/
  virtual void onMeshChanged();

  /**
   * Cache information about what elements were refined and coarsened in the previous step.
   */
  void cacheChangedLists();

  /**
   * Return a range that is suitable for threaded execution over elements that were just refined.
   *
   * @return The _Parent_ elements that are now set to be INACTIVE.  Their _children_ are the new
   * elements.
   */
  ConstElemPointerRange * refinedElementRange() const;

  /**
   * Return a range that is suitable for threaded execution over elements that were just coarsened.
   * Note that these are the _Parent_ elements that are now set to be INACTIVE.  Their _children_
   * are the elements that were just removed.  Use coarsenedElementChildren() to get the element
   * IDs for the children that were just removed for a particular parent element.
   */
  ConstElemPointerRange * coarsenedElementRange() const;

  /**
   * Get the newly removed children element ids for an element that was just coarsened.
   *
   * @param elem Pointer to the parent element that was coarsened to.
   * @return The child element ids in Elem::child() order.
   */
  const std::vector<const Elem *> & coarsenedElementChildren(const Elem * elem) const;

  /**
   * Clears the "semi-local" node list and rebuilds it.  Semi-local nodes
   * consist of all nodes that belong to local and ghost elements.
   */
  void updateActiveSemiLocalNodeRange(std::set<dof_id_type> & ghosted_elems);

  /**
   * Returns true if the node is semi-local
   * @param node Node pointer
   * @return true is the node is semi-local, false otherwise
   */
  bool isSemiLocal(Node * const node) const;

  ///@{
  /**
   * Return pointers to range objects for various types of ranges
   * (local nodes, boundary elems, etc.).
   */
  libMesh::ConstElemRange * getActiveLocalElementRange();
  libMesh::NodeRange * getActiveNodeRange();
  SemiLocalNodeRange * getActiveSemiLocalNodeRange() const;
  libMesh::ConstNodeRange * getLocalNodeRange();
  libMesh::StoredRange<MooseMesh::const_bnd_node_iterator, const BndNode *> *
  getBoundaryNodeRange();
  libMesh::StoredRange<MooseMesh::const_bnd_elem_iterator, const BndElement *> *
  getBoundaryElementRange();
  ///@}

  /**
   * Returns a map of boundaries to ids of elements on the boundary.
   */
  const std::unordered_map<boundary_id_type, std::unordered_set<dof_id_type>> &
  getBoundariesToElems() const;

  /**
   * Returns a map of boundaries to ids of elements on the boundary.
   */
  const std::unordered_map<boundary_id_type, std::unordered_set<dof_id_type>> &
  getBoundariesToActiveSemiLocalElemIds() const;

  /**
   * Return all ids of elements which have a side which is part of a sideset.
   * Note that boundaries are sided.
   * @param bid the id of the sideset of interest
   */
  std::unordered_set<dof_id_type> getBoundaryActiveSemiLocalElemIds(BoundaryID bid) const;

  /**
   * Return all ids of neighbors of elements which have a side which is part of a sideset.
   * Note that boundaries are sided, this is on the neighbor side. For the sideset side, use
   * getBoundariesActiveLocalElemIds.
   * Note that while the element is local and active, the neighbor is not guaranteed to be local,
   * it could be ghosted.
   * Note that if the neighbor is not ghosted, is a remote_elem, then it will not be included
   * @param bid the id of the sideset of interest
   */
  std::unordered_set<dof_id_type> getBoundaryActiveNeighborElemIds(BoundaryID bid) const;

  /**
   * Returns whether a boundary (given by its id) is not crossing through a group of blocks,
   * by which we mean that elements on both sides of the boundary are in those blocks
   * @param bid the id of the boundary of interest
   * @param blk_group the group of blocks potentially traversed
   * @return whether the boundary does not cross between the subdomains in the group
   */
  bool isBoundaryFullyExternalToSubdomains(BoundaryID bid,
                                           const std::set<SubdomainID> & blk_group) const;

  /**
   * Returns a read-only reference to the set of subdomains currently
   * present in the Mesh.
   */
  const std::set<SubdomainID> & meshSubdomains() const;

  /**
   * Returns a read-only reference to the set of boundary IDs currently
   * present in the Mesh.
   */
  const std::set<BoundaryID> & meshBoundaryIds() const;

  /**
   * Returns a read-only reference to the set of sidesets currently
   * present in the Mesh.
   */
  const std::set<BoundaryID> & meshSidesetIds() const;

  /**
   * Returns a read-only reference to the set of nodesets currently
   * present in the Mesh.
   */
  const std::set<BoundaryID> & meshNodesetIds() const;

  /**
   * Sets the mapping between BoundaryID and normal vector
   * Is called by AddAllSideSetsByNormals
   */
  void setBoundaryToNormalMap(std::unique_ptr<std::map<BoundaryID, RealVectorValue>> boundary_map);

  // DEPRECATED METHOD
  void setBoundaryToNormalMap(std::map<BoundaryID, RealVectorValue> * boundary_map);

  /**
   * Sets the set of BoundaryIDs
   * Is called by AddAllSideSetsByNormals
   */
  void setMeshBoundaryIDs(std::set<BoundaryID> boundary_IDs);

  /**
   * Returns the normal vector associated with a given BoundaryID.
   * It's only valid to call this when AddAllSideSetsByNormals is active.
   */
  const RealVectorValue & getNormalByBoundaryID(BoundaryID id) const;

  /**
   * Calls prepare_for_use() if the underlying MeshBase object isn't prepared, then communicates
   * various boundary information on parallel meshes. Also calls update() internally. Instead of
   * calling \p prepare_for_use on the currently held \p MeshBase object, a \p mesh_to_clone can be
   * provided. If it is provided (e.g. this method is given a non-null argument), then \p _mesh will
   * be assigned a clone of the \p mesh_to_clone. The provided \p mesh_to_clone must already be
   * prepared
   * @param mesh_to_clone If nonnull, we will clone this mesh instead of preparing our current one
   * @return Whether the libMesh mesh was prepared. This should really only be relevant in MOOSE
   * framework contexts where we need to make a decision about what to do with the displaced mesh.
   * If the reference mesh base object has \p prepare_for_use called (e.g. this method returns \p
   * true when called for the reference mesh), then we must pass the reference mesh base object into
   * this method when we call this for the displaced mesh. This is because the displaced mesh \emph
   * must be an exact clone of the reference mesh. We have seen that \p prepare_for_use called on
   * two previously identical meshes can result in two different meshes even with Metis partitioning
   */
  bool prepare(const MeshBase * mesh_to_clone);

  /**
   * Calls buildNodeListFromSideList(), buildNodeList(), and buildBndElemList().
   */
  void update();

  /**
   * Returns the level of uniform refinement requested (zero if AMR is disabled).
   */
  unsigned int uniformRefineLevel() const;

  /**
   * Set uniform refinement level
   */
  void setUniformRefineLevel(unsigned int, bool deletion = true);

  /**
   * Return a flag indicating whether or not we should skip remote deletion
   * and repartition after uniform refinements. If the flag is true, uniform
   * refinements will run more efficiently, but at the same time, there might
   * be extra ghosting elements. The number of layers of additional ghosting
   * elements depends on the number of uniform refinement levels.  This flag
   * should be used only when you have a "fine enough" coarse mesh and want
   * to refine the mesh by a few levels. Otherwise, it might introduce an
   * unbalanced workload and too large ghosting domain.
   */
  bool skipDeletionRepartitionAfterRefine() const;

  /**
   * Whether or not skip uniform refinements when using a pre-split mesh
   */
  bool skipRefineWhenUseSplit() const { return _skip_refine_when_use_split; }

  /**
   * This will add the boundary ids to be ghosted to this processor
   */
  void addGhostedBoundary(BoundaryID boundary_id);

  /**
   * This sets the inflation amount for the bounding box for each partition for use in
   * ghosting boundaries
   */
  void setGhostedBoundaryInflation(const std::vector<Real> & inflation);

  /**
   * Return a writable reference to the set of ghosted boundary IDs.
   */
  const std::set<unsigned int> & getGhostedBoundaries() const;

  /**
   * Return a writable reference to the _ghosted_boundaries_inflation vector.
   */
  const std::vector<Real> & getGhostedBoundaryInflation() const;

  /**
   * Actually do the ghosting of boundaries that need to be ghosted to this processor.
   */
  void ghostGhostedBoundaries();

  /**
   * Whether or not we want to ghost ghosted boundaries
   */
  void needGhostGhostedBoundaries(bool needghost) { _need_ghost_ghosted_boundaries = needghost; }

  /**
   * Getter for the patch_size parameter.
   */
  unsigned int getPatchSize() const;

  /**
   * Getter for the ghosting_patch_size parameter.
   */
  unsigned int getGhostingPatchSize() const { return _ghosting_patch_size; }

  /**
   * Getter for the maximum leaf size parameter.
   */
  unsigned int getMaxLeafSize() const { return _max_leaf_size; }

  /**
   * Set the patch size update strategy
   */
  void setPatchUpdateStrategy(Moose::PatchUpdateType patch_update_strategy);

  /**
   * Get the current patch update strategy.
   */
  const Moose::PatchUpdateType & getPatchUpdateStrategy() const;

  /**
   * Get a (slightly inflated) processor bounding box.
   *
   * @param inflation_multiplier This amount will be multiplied by the length of the diagonal of the
   * bounding box to find the amount to inflate the bounding box by in all directions.
   */
  libMesh::BoundingBox getInflatedProcessorBoundingBox(Real inflation_multiplier = 0.01) const;

  /**
   * Implicit conversion operator from MooseMesh -> libMesh::MeshBase.
   */
  operator libMesh::MeshBase &();
  operator const libMesh::MeshBase &() const;

  /**
   * Accessor for the underlying libMesh Mesh object.
   */
  MeshBase & getMesh();
  MeshBase & getMesh(const std::string & name);
  const MeshBase & getMesh() const;
  const MeshBase & getMesh(const std::string & name) const;
  const MeshBase * getMeshPtr() const;

  /**
   * Accessor for Kokkos mesh object.
   */
#ifdef MOOSE_KOKKOS_ENABLED
  const Moose::Kokkos::Mesh * getKokkosMesh() const { return _kokkos_mesh.get(); }
#endif

  /**
   * Calls print_info() on the underlying Mesh.
   */
  void printInfo(std::ostream & os = libMesh::out, const unsigned int verbosity = 0) const;

  /**
   * Return list of blocks to which the given node belongs.
   */
  const std::set<SubdomainID> & getNodeBlockIds(const Node & node) const;

  /**
   * Return a writable reference to a vector of node IDs that belong
   * to nodeset_id.
   */
  const std::vector<dof_id_type> & getNodeList(boundary_id_type nodeset_id) const;

  /**
   * Add a new node to the mesh.  If there is already a node located at the point passed
   * then the node will not be added.  In either case a reference to the node at that location
   * will be returned
   */
  const Node * addUniqueNode(const Point & p, Real tol = 1e-6);

  /**
   * Adds a fictitious "QuadratureNode".  This doesn't actually add it to the libMesh mesh...
   * we just keep track of these here in MooseMesh.
   *
   * QuadratureNodes are fictitious "Nodes" that are located at quadrature points.  This is useful
   * for using the geometric search system to do searches based on quadrature point locations....
   *
   * @param elem The element
   * @param side The side number on which we want to add a quadrature node
   * @param qp The number of the quadrature point
   * @param bid The boundary ID for the point to be added with
   * @param point The physical location of the point
   */
  Node * addQuadratureNode(const Elem * elem,
                           const unsigned short int side,
                           const unsigned int qp,
                           BoundaryID bid,
                           const Point & point);

  /**
   * Get a specified quadrature node.
   *
   * @param elem The element the quadrature point is on
   * @param side The side the quadrature point is on
   * @param qp The quadrature point number associated with the point
   */
  Node * getQuadratureNode(const Elem * elem, const unsigned short int side, const unsigned int qp);

  /**
   * Clear out any existing quadrature nodes.
   * Most likely called before re-adding them.
   */
  void clearQuadratureNodes();

  /**
   * Get the associated BoundaryID for the boundary name.
   *
   * @param boundary_name The name of the boundary.
   * @return the boundary id from the passed boundary name.
   */
  BoundaryID getBoundaryID(const BoundaryName & boundary_name) const;

  /**
   * Get the associated BoundaryID for the boundary names that are passed in.
   *
   * @param boundary_name The names of the boundaries.
   * @return the boundary ids from the passed boundary names.
   */
  std::vector<BoundaryID> getBoundaryIDs(const std::vector<BoundaryName> & boundary_name,
                                         bool generate_unknown = false) const;

  /**
   * Get the associated subdomain ID for the subdomain name.
   *
   * @param subdomain_name The name of the subdomain
   * @return The subdomain id from the passed subdomain name.
   */
  SubdomainID getSubdomainID(const SubdomainName & subdomain_name) const;

  /**
   * Get the associated subdomainIDs for the subdomain names that are passed in.
   *
   * @param subdomain_names The names of the subdomains
   * @return The subdomain ids from the passed subdomain names.
   */
  std::vector<SubdomainID>
  getSubdomainIDs(const std::vector<SubdomainName> & subdomain_names) const;
  std::set<SubdomainID> getSubdomainIDs(const std::set<SubdomainName> & subdomain_names) const;

  /**
   * This method sets the name for \p subdomain_id to \p name
   */
  void setSubdomainName(SubdomainID subdomain_id, const SubdomainName & name);

  /**
   * This method sets the name for \p subdomain_id on the provided \p mesh to \p name
   */
  static void
  setSubdomainName(MeshBase & mesh, SubdomainID subdomain_id, const SubdomainName & name);

  /**
   * Return the name of a block given an id.
   */
  const std::string & getSubdomainName(SubdomainID subdomain_id) const;

  /**
   * Get the associated subdomainNames for the subdomain ids that are passed in.
   *
   * @param subdomain_ids The ids of the subdomains
   * @return The subdomain names from the passed subdomain ids.
   */
  std::vector<SubdomainName>
  getSubdomainNames(const std::vector<SubdomainID> & subdomain_ids) const;

  /**
   * This method sets the boundary name of the boundary based on the id parameter
   */
  void setBoundaryName(BoundaryID boundary_id, BoundaryName name);

  /**
   * Return the name of the boundary given the id.
   */
  const std::string & getBoundaryName(BoundaryID boundary_id) const;

  /**
   * This routine builds a multimap of boundary ids to matching boundary ids across all periodic
   * boundaries
   * in the system.
   */
  void buildPeriodicNodeMap(std::multimap<dof_id_type, dof_id_type> & periodic_node_map,
                            unsigned int var_number,
                            libMesh::PeriodicBoundaries * pbs) const;

  /**
   * This routine builds a datastructure of node ids organized by periodic boundary ids
   */
  void buildPeriodicNodeSets(std::map<BoundaryID, std::set<dof_id_type>> & periodic_node_sets,
                             unsigned int var_number,
                             libMesh::PeriodicBoundaries * pbs) const;

  /**
   * Returns the width of the requested dimension
   */
  Real dimensionWidth(unsigned int component) const;

  ///@{
  /**
   * Returns the min or max of the requested dimension respectively
   */
  virtual Real getMinInDimension(unsigned int component) const;
  virtual Real getMaxInDimension(unsigned int component) const;
  ///@}

  /**
   * This routine determines whether the Mesh is a regular orthogonal mesh (i.e. square in 2D, cubic
   * in 3D). If it is, then we can use a number of convenience functions when periodic boundary
   * conditions are applied.  This routine populates the _range vector which is necessary for these
   * convenience functions.
   *
   * Note:  This routine can potentially identify meshes with concave faces that still "fit" in the
   * convex hull of the corresponding regular orthogonal mesh.  This case is highly unlikely in
   * practice and if a user does this, well.... release the kicker!
   */
  bool detectOrthogonalDimRanges(Real tol = 1e-6);

  /**
   * For "regular orthogonal" meshes, determine if variable var_num is periodic with respect to the
   * primary and secondary BoundaryIDs, record this fact in the _periodic_dim data structure.
   */
  void addPeriodicVariable(unsigned int var_num, BoundaryID primary, BoundaryID secondary);

  /**
   * Returns whether this generated mesh is periodic in the given dimension for the given variable.
   * @param nonlinear_var_num - The nonlinear variable number
   * @param component - An integer representing the desired component (dimension)
   */
  bool isTranslatedPeriodic(unsigned int nonlinear_var_num, unsigned int component) const;

  /**
   * This function returns the minimum vector between two points on the mesh taking into account
   * periodicity for the given variable number.
   * @param nonlinear_var_num - The nonlinear variable number
   * @param p, q - The points between which to compute a minimum vector
   * @return RealVectorValue - The vector pointing from p to q
   */
  RealVectorValue minPeriodicVector(unsigned int nonlinear_var_num, Point p, Point q) const;

  /**
   * This function returns the distance between two points on the mesh taking into account
   * periodicity for the given variable number.
   * @param nonlinear_var_num - The nonlinear variable number
   * @param p, q - The points for which to compute a minimum distance
   * @return Real - The L2 distance between p and q
   */
  Real minPeriodicDistance(unsigned int nonlinear_var_num, Point p, Point q) const;

  /**
   * This function attempts to return the paired boundary ids for the given component.  For example,
   * in a generated 2D mesh, passing 0 for the "x" component will return (3, 1).
   * @param component - An integer representing the desired component (dimension)
   * @return std::pair pointer - The matching boundary pairs for the passed component
   */
  const std::pair<BoundaryID, BoundaryID> * getPairedBoundaryMapping(unsigned int component);

  /**
   * Create the refinement and coarsening maps necessary for projection of stateful material
   * properties when using adaptivity.
   *
   * @param assembly Pointer to the Assembly object for this Mesh.
   */
  void buildRefinementAndCoarseningMaps(Assembly * assembly);

  /**
   * Get the refinement map for a given element type.  This will tell you what quadrature points
   * to copy from and to for stateful material properties on newly created elements from Adaptivity.
   *
   * @param elem The element that represents the element type you need the refinement map for.
   * @param parent_side The side of the parent to map (-1 if not mapping parent sides)
   * @param child The child number (-1 if not mapping child internal sides)
   * @param child_side The side number of the child (-1 if not mapping sides)
   */
  const std::vector<std::vector<QpMap>> &
  getRefinementMap(const Elem & elem, int parent_side, int child, int child_side);

  /**
   * Get the coarsening map for a given element type.  This will tell you what quadrature points
   * to copy from and to for stateful material properties on newly created elements from Adaptivity.
   *
   * @param elem The element that represents the element type you need the coarsening map for.
   * @param input_side The side to map
   */
  const std::vector<std::pair<unsigned int, QpMap>> & getCoarseningMap(const Elem & elem,
                                                                       int input_side);

  /**
   * Change all the boundary IDs for a given side from old_id to new_id.  If delete_prev is true,
   * also actually remove the side with old_id from the BoundaryInfo object.
   */
  void
  changeBoundaryId(const boundary_id_type old_id, const boundary_id_type new_id, bool delete_prev);

  /**
   * Change all the boundary IDs for a given side from old_id to new_id for the given \p mesh.  If
   * delete_prev is true, also actually remove the side with old_id from the BoundaryInfo object.
   */
  static void changeBoundaryId(MeshBase & mesh,
                               const boundary_id_type old_id,
                               const boundary_id_type new_id,
                               bool delete_prev);

  /**
   * Get the list of boundary ids associated with the given subdomain id.
   *
   * @param subdomain_id The subdomain ID you want to get the boundary ids for.
   * @return All boundary IDs connected to elements in the give
   */
  const std::set<BoundaryID> & getSubdomainBoundaryIds(const SubdomainID subdomain_id) const;

  /**
   * Get the list of boundaries that contact the given subdomain.
   *
   * @param subdomain_id The subdomain ID you want to get the boundary ids for.
   * @return All boundary IDs connected to elements in the given subdomain
   */
  std::set<BoundaryID> getSubdomainInterfaceBoundaryIds(const SubdomainID subdomain_id) const;

  /**
   * Get the list of subdomains associated with the given boundary.
   *
   * @param bid The boundary ID you want to get the subdomain IDs for.
   * @return All subdomain IDs associated with given boundary ID
   */
  std::set<SubdomainID> getBoundaryConnectedBlocks(const BoundaryID bid) const;

  /**
   * Get the list of subdomains associated with the given boundary of its secondary side.
   *
   * @param bid The boundary ID you want to get the subdomain IDs for.
   * @return All subdomain IDs associated with given boundary ID
   */
  std::set<SubdomainID> getBoundaryConnectedSecondaryBlocks(const BoundaryID bid) const;

  /**
   * Get the list of subdomains contacting the given boundary.
   *
   * @param bid The boundary ID you want to get the subdomain IDs for.
   * @return All subdomain IDs contacting given boundary ID
   */
  std::set<SubdomainID> getInterfaceConnectedBlocks(const BoundaryID bid) const;

  /**
   * Get the list of subdomains neighboring a given subdomain.
   *
   * @param subdomain_id The boundary ID you want to get the subdomain IDs for.
   * @return All subdomain IDs neighboring a given subdomain
   */
  const std::set<SubdomainID> & getBlockConnectedBlocks(const SubdomainID subdomain_id) const;

  /**
   * Returns true if the requested node is in the list of boundary nodes, false otherwise.
   */
  bool isBoundaryNode(dof_id_type node_id) const;

  /**
   * Returns true if the requested node is in the list of boundary nodes for the specified boundary,
   * false otherwise.
   */
  bool isBoundaryNode(dof_id_type node_id, BoundaryID bnd_id) const;

  /**
   * Returns true if the requested element is in the list of boundary elements, false otherwise.
   */
  bool isBoundaryElem(dof_id_type elem_id) const;

  /**
   * Returns true if the requested element is in the list of boundary elements for the specified
   * boundary, false otherwise.
   */
  bool isBoundaryElem(dof_id_type elem_id, BoundaryID bnd_id) const;

  /**
   * Generate a unified error message if the underlying libMesh mesh is a DistributedMesh.  Clients
   * of MooseMesh can use this function to throw an error if they know they don't work with
   * DistributedMesh.
   *
   * See, for example, the NodalVariableValue class.
   */
  void errorIfDistributedMesh(std::string name) const;

  /**
   * Returns the final Mesh distribution type.
   */
  virtual bool isDistributedMesh() const { return _use_distributed_mesh; }

  /**
   * Tell the user if the distribution was overriden for any reason
   */
  bool isParallelTypeForced() const { return _parallel_type_overridden; }

  /**
   *  Allow to change parallel type
   */
  void setParallelType(ParallelType parallel_type);

  /**
   * @return The parallel type
   */
  ParallelType getParallelType() const { return _parallel_type; }

  /*
   * Set/Get the partitioner name
   */
  const MooseEnum & partitionerName() const { return _partitioner_name; }

  /**
   * Tell the user if the partitioner was overriden for any reason
   */
  bool isPartitionerForced() const { return _partitioner_overridden; }

  /**
   * Set whether or not this mesh is allowed to read a recovery file.
   */
  void allowRecovery(bool allow) { _allow_recovery = allow; }

  /**
   * Method for setting the partitioner on the passed in mesh_base object.
   */
  static void setPartitioner(MeshBase & mesh_base,
                             MooseEnum & partitioner,
                             bool use_distributed_mesh,
                             const InputParameters & params,
                             MooseObject & context_obj);

  /**
   * Setter for custom partitioner
   */
  void setCustomPartitioner(libMesh::Partitioner * partitioner);

  ///@{
  /**
   * Setter and getter for _custom_partitioner_requested
   */
  bool isCustomPartitionerRequested() const;
  void setIsCustomPartitionerRequested(bool cpr);
  ///@}

  /// Getter to query if the mesh was detected to be regular and orthogonal
  bool isRegularOrthogonal() { return _regular_orthogonal_mesh; }

  /// check if the mesh has SECOND order elements
  bool hasSecondOrderElements();

  /**
   * Proxy function to get a (sub)PointLocator from either the underlying libMesh mesh (default), or
   * to allow derived meshes to return a custom point locator.
   */
  virtual std::unique_ptr<libMesh::PointLocatorBase> getPointLocator() const;

  /**
   * Returns the name of the mesh file read to produce this mesh if any or an empty string
   * otherwise.
   */
  virtual std::string getFileName() const { return ""; }

  /// Helper type for building periodic node maps
  using PeriodicNodeInfo = std::pair<const Node *, BoundaryID>;

  /**
   * Set whether we need to delete remote elements
   */
  void needsRemoteElemDeletion(bool need_delete) { _need_delete = need_delete; }

  /**
   * Whether we need to delete remote elements
   */
  bool needsRemoteElemDeletion() const { return _need_delete; }

  /**
   * Set whether to allow remote element removal
   */
  void allowRemoteElementRemoval(bool allow_removal);

  /**
   * Whether we are allow remote element removal
   */
  bool allowRemoteElementRemoval() const { return _allow_remote_element_removal; }

  /**
   * Delete remote elements
   */
  void deleteRemoteElements();

  /**
   * Whether mesh base object was constructed or not
   */
  bool hasMeshBase() const { return _mesh.get() != nullptr; }

  /**
   * Whether mesh has an extra element integer with a given name
   */
  bool hasElementID(const std::string & id_name) const;

  /**
   * Return the accessing integer for an extra element integer with its name
   */
  unsigned int getElementIDIndex(const std::string & id_name) const;

  /**
   * Return the maximum element ID for an extra element integer with its accessing index
   */
  dof_id_type maxElementID(unsigned int elem_id_index) const { return _max_ids[elem_id_index]; }

  /**
   * Return the minimum element ID for an extra element integer with its accessing index
   */
  dof_id_type minElementID(unsigned int elem_id_index) const { return _min_ids[elem_id_index]; }

  /**
   * Whether or not two extra element integers are identical
   */
  bool areElemIDsIdentical(const std::string & id_name1, const std::string & id_name2) const;

  /**
   * Return all the unique element IDs for an extra element integer with its index
   */
  std::set<dof_id_type> getAllElemIDs(unsigned int elem_id_index) const;

  /**
   * Return all the unique element IDs for an extra element integer with its index on a set of
   * subdomains
   */
  std::set<dof_id_type> getElemIDsOnBlocks(unsigned int elem_id_index,
                                           const std::set<SubdomainID> & blks) const;

  /**
   * Get the maximum number of sides per element
   */
  unsigned int getMaxSidesPerElem() const { return _max_sides_per_elem; }

  /**
   * Get the maximum number of nodes per element
   */
  unsigned int getMaxNodesPerElem() const { return _max_nodes_per_elem; }

  /**
   * Get the maximum number of nodes per side
   */
  unsigned int getMaxNodesPerSide() const { return _max_nodes_per_side; }

  std::unordered_map<dof_id_type, std::set<dof_id_type>>
  getElemIDMapping(const std::string & from_id_name, const std::string & to_id_name) const;

  ///@{ accessors for the FaceInfo objects
  unsigned int nFace() const { return _face_info.size(); }

  /// Accessor for local \p FaceInfo objects.
  const std::vector<const FaceInfo *> & faceInfo() const;

  /// Need to declare these iterators here to make sure the iterators below work
  struct face_info_iterator;
  struct const_face_info_iterator;

  /// Iterators to owned faceInfo objects. These faceInfo-s are required for the
  /// face loops and to filter out the faceInfo-s that are not owned by this processor
  /// in case we have a distributed mesh and we included FaceInfo objects that
  /// are on processor boundaries
  face_info_iterator ownedFaceInfoBegin();
  face_info_iterator ownedFaceInfoEnd();

  /// Need to declare these iterators here to make sure the iterators below work
  struct elem_info_iterator;
  struct const_elem_info_iterator;

  /// Iterators to owned faceInfo objects. These faceInfo-s are required for the
  /// face loops and to filter out the faceInfo-s that are not owned by this processor
  /// in case we have a distributed mesh and we included FaceInfo objects that
  /// are on processor boundaries
  elem_info_iterator ownedElemInfoBegin();
  elem_info_iterator ownedElemInfoEnd();

  /// Accessor for the local FaceInfo object on the side of one element. Returns null if ghosted.
  const FaceInfo * faceInfo(const Elem * elem, unsigned int side) const;

  /// Accessor for the elemInfo object for a given element ID
  const ElemInfo & elemInfo(const dof_id_type id) const;

  /// Accessor for the element info objects owned by this process
  const std::vector<const ElemInfo *> & elemInfoVector() const { return _elem_info; }

  /// Accessor for all \p FaceInfo objects.
  const std::vector<FaceInfo> & allFaceInfo() const;
  ///@}

  /**
   * Cache if variables live on the elements connected by the FaceInfo objects
   */
  void cacheFaceInfoVariableOwnership() const;

  /**
   * Cache the DoF indices for FV variables on each element. These indices are used to speed up the
   * setup loops of finite volume systems.
   */
  void cacheFVElementalDoFs() const;

  /**
   * Compute the face coordinate value for all \p FaceInfo and \p ElemInfo objects. 'Coordinate'
   * here means a coordinate value associated with the coordinate system. For Cartesian coordinate
   * systems, 'coordinate' is simply '1'; in RZ, '2*pi*r', and in spherical, '4*pi*r^2'
   */
  void computeFiniteVolumeCoords() const;

  /**
   * Set whether this mesh is a displaced mesh
   */
  void isDisplaced(bool is_displaced) { _is_displaced = is_displaced; }

  /**
   * whether this mesh is a displaced mesh
   */
  bool isDisplaced() const { return _is_displaced; }

  /**
   * @return A map from nodeset ids to the vector of node ids in the nodeset
   */
  const std::map<boundary_id_type, std::vector<dof_id_type>> & nodeSetNodes() const;

  /**
   * Get the coordinate system type, e.g. xyz, rz, or r-spherical, for the provided subdomain id \p
   * sid
   */
  Moose::CoordinateSystemType getCoordSystem(SubdomainID sid) const;

  /**
   * Get the coordinate system from the mesh, it must be the same in all subdomains otherwise this
   * will error
   */
  Moose::CoordinateSystemType getUniqueCoordSystem() const;

  /**
   * Get the map from subdomain ID to coordinate system type, e.g. xyz, rz, or r-spherical
   */
  const std::map<SubdomainID, Moose::CoordinateSystemType> & getCoordSystem() const;

  /**
   * Set the coordinate system for the provided blocks to \p coord_sys
   */
  void setCoordSystem(const std::vector<SubdomainName> & blocks, const MultiMooseEnum & coord_sys);

  /**
   * For axisymmetric simulations, set the symmetry coordinate axis. For r in the x-direction, z in
   * the y-direction the coordinate axis would be y
   */
  void setAxisymmetricCoordAxis(const MooseEnum & rz_coord_axis);

  /**
   * Sets the general coordinate axes for axisymmetric blocks.
   *
   * This method must be used if any of the following are true:
   * - There are multiple axisymmetric coordinate systems
   * - Any axisymmetric coordinate system axis/direction is not the +X or +Y axis
   * - Any axisymmetric coordinate system does not start at (0,0,0)
   *
   * @param[in] blocks  Subdomain names
   * @param[in] axes  Pair of values defining the axisymmetric coordinate axis
   *                  for each subdomain. The first value is the point on the axis
   *                  corresponding to the origin. The second value is the direction
   *                  vector of the axis (normalization not necessary).
   */
  void setGeneralAxisymmetricCoordAxes(const std::vector<SubdomainName> & blocks,
                                       const std::vector<std::pair<Point, RealVectorValue>> & axes);

  /**
   * Gets the general axisymmetric coordinate axis for a block.
   *
   * @param[in] subdomain_id  Subdomain ID for which to get axisymmetric coordinate axis
   */
  const std::pair<Point, RealVectorValue> &
  getGeneralAxisymmetricCoordAxis(SubdomainID subdomain_id) const;

  /**
   * Returns true if general axisymmetric coordinate axes are being used
   */
  bool usingGeneralAxisymmetricCoordAxes() const;

  /**
   * Returns the desired radial direction for RZ coordinate transformation
   * @return The coordinate direction for the radial direction
   */
  unsigned int getAxisymmetricRadialCoord() const;

  /**
   * Performs a sanity check for every element in the mesh. If an element dimension is 3 and the
   * corresponding coordinate system is RZ, then this will error. If an element dimension is greater
   * than 1 and the corresponding system is RPSHERICAL then this will error
   */
  void checkCoordinateSystems();

  /**
   * Set the coordinate system data to that of \p other_mesh
   */
  void setCoordData(const MooseMesh & other_mesh);

  /**
   * Mark the finite volume information as dirty
   */
  void markFiniteVolumeInfoDirty() { _finite_volume_info_dirty = true; }

  /**
   * @return whether the finite volume information is dirty
   */
  bool isFiniteVolumeInfoDirty() const { return _finite_volume_info_dirty; }

  /**
   * @return the coordinate transformation object that describes how to transform this problem's
   * coordinate system into the canonical/reference coordinate system
   */
  MooseAppCoordTransform & coordTransform();

  /**
   * @return the length unit of this mesh provided through the coordinate transformation object
   */
  const MooseUnits & lengthUnit() const;

  /**
   * This function attempts to return the map from a high-order element side to its corresponding
   * lower-d element
   */
  const std::unordered_map<std::pair<const Elem *, unsigned short int>, const Elem *> &
  getLowerDElemMap() const;

  /**
   * @return Whether or not this mesh comes from a split mesh
   */
  bool isSplit() const { return _is_split; }

  /**
   * Builds the face and elem info vectors that store meta-data needed for looping over and doing
   * calculations based on mesh faces and elements in a finite volume setting. This should only
   * be called when finite volume variables are used in the problem or when the face and elem info
   * objects are necessary for functor-based evaluations.
   */
  void buildFiniteVolumeInfo() const;

  /**
   * Sets up the additional data needed for finite volume computations.
   * This involves building FaceInfo and ElemInfo objects, caching variable associations
   * and elemental DoF indices for FV variables.
   */
  void setupFiniteVolumeMeshData() const;

  /**
   * Indicate whether the kind of adaptivity we're doing is p-refinement
   */
  void doingPRefinement(bool doing_p_refinement) { _doing_p_refinement = doing_p_refinement; }

  /**
   * Query whether we have p-refinement
   */
  [[nodiscard]] bool doingPRefinement() const { return _doing_p_refinement; }

  /**
   * Returns the maximum p-refinement level of all elements
   */
  unsigned int maxPLevel() const { return _max_p_level; }

  /**
   * Returns the maximum h-refinement level of all elements
   */
  unsigned int maxHLevel() const { return _max_h_level; }

  /**
   * Get the map describing for each volumetric quadrature point (qp) on the refined level which qp
   * on the previous coarser level the fine qp is closest to
   */
  const std::vector<QpMap> & getPRefinementMap(const Elem & elem) const;
  /**
   * Get the map describing for each side quadrature point (qp) on the refined level which qp
   * on the previous coarser level the fine qp is closest to
   */
  const std::vector<QpMap> & getPRefinementSideMap(const Elem & elem) const;
  /**
   * Get the map describing for each volumetric quadrature point (qp) on the coarse level which qp
   * on the previous finer level the coarse qp is closest to
   */
  const std::vector<QpMap> & getPCoarseningMap(const Elem & elem) const;
  /**
   * Get the map describing for each side quadrature point (qp) on the coarse level which qp
   * on the previous finer level the coarse qp is closest to
   */
  const std::vector<QpMap> & getPCoarseningSideMap(const Elem & elem) const;

  void buildPRefinementAndCoarseningMaps(Assembly * assembly);

  /**
   * @return Whether the subdomain indicated by \p subdomain_id is a lower-dimensional manifold of
   * some higher-dimensional subdomain, or in implementation speak, whether the elements of this
   * subdomain have non-null interior parents
   */
  bool isLowerD(const SubdomainID subdomain_id) const;

  /**
   * @return Whether there are any lower-dimensional blocks that are manifolds of higher-dimensional
   * block faces
   */
  bool hasLowerD() const { return _has_lower_d; }

  /**
   * @return The set of lower-dimensional blocks for interior sides
   */
  const std::set<SubdomainID> & interiorLowerDBlocks() const { return _lower_d_interior_blocks; }
  /**
   * @return The set of lower-dimensional blocks for boundary sides
   */
  const std::set<SubdomainID> & boundaryLowerDBlocks() const { return _lower_d_boundary_blocks; }
  /// Return construct node list from side list boolean
  bool getConstructNodeListFromSideList() { return _construct_node_list_from_side_list; }

  /// Add a pair of disconnected neighbors
  void addDisconnectedNeighbors(const ConstBndElement & bndelem1, const ConstBndElement & bndelem2)
  {
    _disconnected_neighbors.emplace(bndelem1, bndelem2);
  }

  /**
   * @brief Check if there is a disconnected neighbor for the given element and side
   *
   * @return nullptr if no disconnected neighbor exists
   */
  const ConstBndElement * disconnectedNeighbor(const Elem * elem, unsigned int side) const
  {
    if (_disconnected_neighbors.empty())
      return nullptr;

    for (const auto & [bndelem1, bndelem2] : _disconnected_neighbors)
    {
      if (bndelem1.elem == elem && bndelem1.side == side)
        return &bndelem2;
      if (bndelem2.elem == elem && bndelem2.side == side)
        return &bndelem1;
    }
    return nullptr;
  }

protected:
  /// Deprecated (DO NOT USE)
  std::vector<std::unique_ptr<libMesh::GhostingFunctor>> _ghosting_functors;

  /// The list of active geometric relationship managers (bound to the underlying MeshBase object).
  std::vector<std::shared_ptr<RelationshipManager>> _relationship_managers;

  /// Whether or not this mesh was built from another mesh
  bool _built_from_other_mesh = false;

  /// Can be set to DISTRIBUTED, REPLICATED, or DEFAULT.  Determines whether
  /// the underlying libMesh mesh is a ReplicatedMesh or DistributedMesh.
  ParallelType _parallel_type;

  /// False by default.  Final value is determined by several factors
  /// including the 'distribution' setting in the input file, and whether
  /// or not the Mesh file is a Nemesis file.
  bool _use_distributed_mesh;
  bool _distribution_overridden;
  bool _parallel_type_overridden;

  /// Pointer to underlying libMesh mesh object
  std::unique_ptr<libMesh::MeshBase> _mesh;

  /// Pointer to Kokkos mesh object
#ifdef MOOSE_KOKKOS_ENABLED
  std::unique_ptr<Moose::Kokkos::Mesh> _kokkos_mesh;
#endif

  /// The partitioner used on this mesh
  MooseEnum _partitioner_name;
  bool _partitioner_overridden;

  /// The custom partitioner
  std::unique_ptr<libMesh::Partitioner> _custom_partitioner;
  bool _custom_partitioner_requested;

  /// Convenience enums
  enum
  {
    X = 0,
    Y,
    Z
  };
  enum
  {
    MIN = 0,
    MAX
  };

  /// The level of uniform refinement requested (set to zero if AMR is disabled)
  unsigned int _uniform_refine_level;

  /// Whether or not to skip uniform refinements when using a pre-split mesh
  bool _skip_refine_when_use_split;

  /// Whether or not skip remote deletion and repartition after uniform refinements
  bool _skip_deletion_repartition_after_refine;

  /// true if mesh is changed (i.e. after adaptivity step)
  bool _is_changed;

  /// True if a Nemesis Mesh was read in
  bool _is_nemesis;

  /// True if prepare has been called on the mesh
  bool _moose_mesh_prepared = false;

  /// The elements that were just refined.
  std::unique_ptr<ConstElemPointerRange> _refined_elements;

  /// The elements that were just coarsened.
  std::unique_ptr<ConstElemPointerRange> _coarsened_elements;

  /**
   * Map of Parent elements to child elements for elements that were just coarsened.
   *
   * NOTE: the child element pointers ARE PROBABLY INVALID.  Only use them for indexing!
   */
  std::map<const Elem *, std::vector<const Elem *>> _coarsened_element_children;

  /// Used for generating the semilocal node range
  std::set<Node *> _semilocal_node_list;

  /**
   * A range for use with threading.  We do this so that it doesn't have
   * to get rebuilt all the time (which takes time).
   */
  std::unique_ptr<libMesh::ConstElemRange> _active_local_elem_range;

  std::unique_ptr<SemiLocalNodeRange> _active_semilocal_node_range;
  std::unique_ptr<libMesh::NodeRange> _active_node_range;
  std::unique_ptr<libMesh::ConstNodeRange> _local_node_range;
  std::unique_ptr<libMesh::StoredRange<MooseMesh::const_bnd_node_iterator, const BndNode *>>
      _bnd_node_range;
  std::unique_ptr<libMesh::StoredRange<MooseMesh::const_bnd_elem_iterator, const BndElement *>>
      _bnd_elem_range;

  /// A map of all of the current nodes to the elements that they are connected to.
  std::map<dof_id_type, std::vector<dof_id_type>> _node_to_elem_map;
  bool _node_to_elem_map_built;

  /// A map of all of the current nodes to the active elements that they are connected to.
  std::map<dof_id_type, std::vector<dof_id_type>> _node_to_active_semilocal_elem_map;
  bool _node_to_active_semilocal_elem_map_built;

  /**
   * A set of subdomain IDs currently present in the mesh. For parallel meshes, includes
   * subdomains defined on other processors as well.
   */
  std::set<SubdomainID> _mesh_subdomains;

  ///@{
  /**
   * A set of boundary IDs currently present in the mesh. In serial, this is equivalent to the
   * values returned by _mesh.get_boundary_info().get_boundary_ids(). In parallel, it will contain
   * off-processor boundary IDs as well.
   */
  std::set<BoundaryID> _mesh_boundary_ids;
  std::set<BoundaryID> _mesh_sideset_ids;
  std::set<BoundaryID> _mesh_nodeset_ids;
  ///@}

  /// The boundary to normal map - valid only when AddAllSideSetsByNormals is active
  std::unique_ptr<std::map<BoundaryID, RealVectorValue>> _boundary_to_normal_map;

  /// array of boundary nodes
  std::vector<BndNode *> _bnd_nodes;
  typedef std::vector<BndNode *>::iterator bnd_node_iterator_imp;
  typedef std::vector<BndNode *>::const_iterator const_bnd_node_iterator_imp;
  /// Map of sets of node IDs in each boundary
  std::map<boundary_id_type, std::set<dof_id_type>> _bnd_node_ids;

  /// array of boundary elems
  std::vector<BndElement *> _bnd_elems;
  typedef std::vector<BndElement *>::iterator bnd_elem_iterator_imp;
  typedef std::vector<BndElement *>::const_iterator const_bnd_elem_iterator_imp;

  /// Map of set of elem IDs connected to each boundary
  std::unordered_map<boundary_id_type, std::unordered_set<dof_id_type>> _bnd_elem_ids;

  std::map<dof_id_type, Node *> _quadrature_nodes;
  std::map<dof_id_type, std::map<unsigned int, std::map<dof_id_type, Node *>>>
      _elem_to_side_to_qp_to_quadrature_nodes;
  std::vector<BndNode> _extra_bnd_nodes;

  /// list of nodes that belongs to a specified block (domain)
  std::map<dof_id_type, std::set<SubdomainID>> _block_node_list;

  /// list of nodes that belongs to a specified nodeset: indexing [nodeset_id] -> [array of node ids]
  std::map<boundary_id_type, std::vector<dof_id_type>> _node_set_nodes;

  std::set<unsigned int> _ghosted_boundaries;
  std::vector<Real> _ghosted_boundaries_inflation;

  /// The number of nodes to consider in the NearestNode neighborhood.
  unsigned int _patch_size;

  /// The number of nearest neighbors to consider for ghosting purposes when iteration patch update strategy is used.
  unsigned int _ghosting_patch_size;

  // The maximum number of points in each leaf of the KDTree used in the nearest neighbor search.
  unsigned int _max_leaf_size;

  /// The patch update strategy
  Moose::PatchUpdateType _patch_update_strategy;

  /// Vector of all the Nodes in the mesh for determining when to add a new point
  std::vector<Node *> _node_map;

  /// Boolean indicating whether this mesh was detected to be regular and orthogonal
  bool _regular_orthogonal_mesh;

  /// The bounds in each dimension of the mesh for regular orthogonal meshes
  std::vector<std::vector<Real>> _bounds;

  /// A vector holding the paired boundaries for a regular orthogonal mesh
  std::vector<std::pair<BoundaryID, BoundaryID>> _paired_boundary;

  /// Whether or not we are using a (pre-)split mesh (automatically DistributedMesh)
  const bool _is_split;

  /**
   * @brief List of neighbor pairs that are not topologically connected
   *
   * libmesh keeps track of element point/edge/face neighbors that are topologically connected. This
   * data structure maintains additional element neighbors that are potentially topologically
   * disconnected. In certain cases, constraints or weak forms (e.g., cohesive zone
   * traction-separation models) may be applied across the "interfaces" defined by these neighbor
   * pairs.
   */
  std::unordered_set<std::pair<ConstBndElement, ConstBndElement>> _disconnected_neighbors;

  void cacheInfo();
  void freeBndNodes();
  void freeBndElems();
  void setPartitionerHelper(MeshBase * mesh = nullptr);

private:
  /// Map connecting elems with their corresponding ElemInfo, we use the element ID as
  /// the key
  mutable std::unordered_map<dof_id_type, ElemInfo> _elem_to_elem_info;

  /// Holds only those \p ElemInfo objects that have \p processor_id equal to this process's id,
  /// e.g. the local \p ElemInfo objects
  mutable std::vector<const ElemInfo *> _elem_info;

  /// FaceInfo object storing information for face based loops. This container holds all the \p
  /// FaceInfo objects accessible from this process
  mutable std::vector<FaceInfo> _all_face_info;

  /// Holds only those \p FaceInfo objects that have \p processor_id equal to this process's id,
  /// e.g. the local \p FaceInfo objects
  mutable std::vector<const FaceInfo *> _face_info;

  /// Map from elem-side pair to FaceInfo
  mutable std::unordered_map<std::pair<const Elem *, unsigned int>, FaceInfo *>
      _elem_side_to_face_info;

  // true if the _face_info member needs to be rebuilt/updated.
  mutable bool _finite_volume_info_dirty = true;

  // True if we have cached elemental dofs ids for the linear finite volume variables.
  // This happens in the first system which has a linear finite volume variable, considering
  // that currently we only support one variable per linear system.
  mutable bool _linear_finite_volume_dofs_cached = false;

  /**
   * A map of vectors indicating which dimensions are periodic in a regular orthogonal mesh for
   * the specified variable numbers.  This data structure is populated by addPeriodicVariable.
   */
  std::map<unsigned int, std::vector<bool>> _periodic_dim;

  /**
   * A convenience vector used to hold values in each dimension representing half of the range.
   */
  RealVectorValue _half_range;

  /// A vector containing the nodes at the corners of a regular orthogonal mesh
  std::vector<Node *> _extreme_nodes;

  /**
   * This routine detects paired sidesets of a regular orthogonal mesh (.i.e. parallel sidesets
   * "across" from one and other).
   * The _paired_boundary datastructure is populated with this information.
   */
  void detectPairedSidesets();

  /**
   * Build the refinement map for a given element type.  This will tell you what quadrature points
   * to copy from and to for stateful material properties on newly created elements from
   * Adaptivity.
   *
   * @param elem The element that represents the element type you need the refinement map for.
   * @param qrule The quadrature rule in use.
   * @param qrule_face The current face quadrature rule
   * @param parent_side The side of the parent to map (-1 if not mapping parent sides)
   * @param child The child number (-1 if not mapping child internal sides)
   * @param child_side The side number of the child (-1 if not mapping sides)
   */
  void buildRefinementMap(const Elem & elem,
                          libMesh::QBase & qrule,
                          libMesh::QBase & qrule_face,
                          int parent_side,
                          int child,
                          int child_side);

  /**
   * Build the coarsening map for a given element type.  This will tell you what quadrature points
   * to copy from and to for stateful material properties on newly created elements from
   * Adaptivity.
   *
   * @param elem The element that represents the element type you need the coarsening map for.
   * @param qrule The quadrature rule in use.
   * @param qrule_face The current face quadrature rule
   * @param input_side The side to map
   */
  void buildCoarseningMap(const Elem & elem,
                          libMesh::QBase & qrule,
                          libMesh::QBase & qrule_face,
                          int input_side);

  /**
   * Find the closest points that map "from" to "to" and fill up "qp_map".
   * Essentially, for each point in "from" find the closest point in "to".
   *
   * @param from The reference positions in the parent of the the points we're mapping _from_
   * @param to The reference positions in the parent of the the points we're mapping _to_
   * @param qp_map This will be filled with QpMap objects holding the mappings.
   */
  void mapPoints(const std::vector<Point> & from,
                 const std::vector<Point> & to,
                 std::vector<QpMap> & qp_map);

  /**
   * Given an elem type, get maps that tell us what qp's are closest to each other between a
   * parent and it's children. This is mainly used for mapping stateful material properties during
   * adaptivity.
   *
   * There are 3 cases here:
   *
   * 1. Volume to volume (parent_side = -1, child = -1, child_side = -1)
   * 2. Parent side to child side (parent_side = 0+, child = -1, child_side = 0+)
   * 3. Child side to parent volume (parent_side = -1, child = 0+, child_side = 0+)
   *
   * Case 3 only happens under refinement (need to invent data at internal child sides).
   *
   * @param template_elem An element of the type that we need to find the maps for
   * @param qrule The quadrature rule that we need to find the maps for
   * @param qrule_face The face quadrature rule that we need to find the maps for
   * @param refinement_map The map to use when an element gets split
   * @param coarsen_map The map to use when an element is coarsened.
   * @param parent_side - the id of the parent's side
   * @param child - the id of the child element
   * @param child_side - The id of the child's side
   */
  void findAdaptivityQpMaps(const Elem * template_elem,
                            libMesh::QBase & qrule,
                            libMesh::QBase & qrule_face,
                            std::vector<std::vector<QpMap>> & refinement_map,
                            std::vector<std::pair<unsigned int, QpMap>> & coarsen_map,
                            int parent_side,
                            int child,
                            int child_side);

  void buildHRefinementAndCoarseningMaps(Assembly * assembly);

  const std::vector<QpMap> & getPRefinementMapHelper(
      const Elem & elem,
      const std::map<std::pair<libMesh::ElemType, unsigned int>, std::vector<QpMap>> &) const;
  const std::vector<QpMap> & getPCoarseningMapHelper(
      const Elem & elem,
      const std::map<std::pair<libMesh::ElemType, unsigned int>, std::vector<QpMap>> &) const;

  /**
   * Update the coordinate transformation object based on our coordinate system data. The
   * coordinate transformation will be created if it hasn't been already
   */
  void updateCoordTransform();

  /**
   * Loop through all subdomain IDs and check if there is name duplication used for the subdomains
   * with same ID. Throw out an error if any name duplication is found.
   */
  void checkDuplicateSubdomainNames();

  /// Holds mappings for volume to volume and parent side to child side
  /// Map key:
  /// - first member corresponds to element side. It's -1 for volume quadrature points
  /// - second member correponds to the element type
  /// Map value:
  /// - Outermost index is the child element index
  /// - Once we have indexed by the child element index, we have a std::vector of QpMaps. This
  ///   vector is sized by the number of reference points in the child element. Then for each
  ///   reference point in the child element we have a QpMap whose \p _from index corresponds to
  ///   the child element reference point, a \p _to index which corresponds to the reference point
  ///   on the parent element that the child element reference point is closest to, and a
  ///   \p _distance member which is the distance between the mapped child and parent reference
  ///   quadrature points
  std::map<std::pair<int, libMesh::ElemType>, std::vector<std::vector<QpMap>>>
      _elem_type_to_refinement_map;

  std::map<std::pair<libMesh::ElemType, unsigned int>, std::vector<QpMap>>
      _elem_type_to_p_refinement_map;
  std::map<std::pair<libMesh::ElemType, unsigned int>, std::vector<QpMap>>
      _elem_type_to_p_refinement_side_map;

  /// Holds mappings for "internal" child sides to parent volume.  The second key is (child, child_side).
  std::map<libMesh::ElemType, std::map<std::pair<int, int>, std::vector<std::vector<QpMap>>>>
      _elem_type_to_child_side_refinement_map;

  /// Holds mappings for volume to volume and parent side to child side
  /// Map key:
  /// - first member corresponds to element side. It's -1 for volume quadrature points
  /// - second member correponds to the element type
  /// Map value:
  /// - Vector is sized based on the number of quadrature points in the parent (e.g. coarser)
  ///   element.
  /// - For each parent quadrature point we store a pair
  ///   - The first member of the pair identifies which child holds the closest refined-level
  ///     quadrature point
  ///   - The second member of the pair is the QpMap. The \p _from data member will correspond to
  ///     the parent quadrature point index. The \p _to data member will correspond to which child
  ///     element quadrature point is closest to the parent quadrature point. And \p _distance is
  ///     the distance between the two
  std::map<std::pair<int, libMesh::ElemType>, std::vector<std::pair<unsigned int, QpMap>>>
      _elem_type_to_coarsening_map;

  std::map<std::pair<libMesh::ElemType, unsigned int>, std::vector<QpMap>>
      _elem_type_to_p_coarsening_map;
  std::map<std::pair<libMesh::ElemType, unsigned int>, std::vector<QpMap>>
      _elem_type_to_p_coarsening_side_map;

  struct SubdomainData
  {
    /// Neighboring subdomain ids
    std::set<SubdomainID> neighbor_subs;

    /// The boundary ids that are attached. This set will include any sideset boundary ID that
    /// is a side of any part of the subdomain
    std::set<BoundaryID> boundary_ids;

    /// Whether this subdomain is a lower-dimensional manifold of a higher-dimensional subdomain
    bool is_lower_d;
  };

  /// Holds a map from subdomain ids to associated data
  std::unordered_map<SubdomainID, SubdomainData> _sub_to_data;

  /// Holds a map from neighbor subomdain ids to the boundary ids that are attached to it
  std::unordered_map<SubdomainID, std::set<BoundaryID>> _neighbor_subdomain_boundary_ids;

  /// Mesh blocks for interior lower-d elements in different types
  std::set<SubdomainID> _lower_d_interior_blocks;
  /// Mesh blocks for boundary lower-d elements in different types
  std::set<SubdomainID> _lower_d_boundary_blocks;
  /// Holds a map from a high-order element side to its corresponding lower-d element
  std::unordered_map<std::pair<const Elem *, unsigned short int>, const Elem *>
      _higher_d_elem_side_to_lower_d_elem;
  std::unordered_map<const Elem *, unsigned short int> _lower_d_elem_to_higher_d_elem_side;

  /// Whether there are any lower-dimensional blocks that are manifolds of higher-dimensional block
  /// faces
  bool _has_lower_d;

  /// Whether or not this Mesh is allowed to read a recovery file
  bool _allow_recovery;

  /// Whether or not to allow generation of nodesets from sidesets
  bool _construct_node_list_from_side_list;

  /// Whether we need to delete remote elements after init'ing the EquationSystems
  bool _need_delete;

  /// Whether to allow removal of remote elements
  bool _allow_remote_element_removal;

  /// Set of elements ghosted by ghostGhostedBoundaries
  std::set<Elem *> _ghost_elems_from_ghost_boundaries;

  /// A parallel mesh generator such as DistributedRectilinearMeshGenerator
  /// already make everything ready. We do not need to gather all boundaries to
  /// every single processor. In general, we should avoid using ghostGhostedBoundaries
  /// when possible since it is not scalable
  bool _need_ghost_ghosted_boundaries;

  /// Unique element integer IDs for each subdomain and each extra element integers
  std::vector<std::unordered_map<SubdomainID, std::set<dof_id_type>>> _block_id_mapping;
  /// Maximum integer ID for each extra element integer
  std::vector<dof_id_type> _max_ids;
  /// Minimum integer ID for each extra element integer
  std::vector<dof_id_type> _min_ids;
  /// Flags to indicate whether or not any two extra element integers are the same
  std::vector<std::vector<bool>> _id_identical_flag;

  /// The maximum number of sides per element
  unsigned int _max_sides_per_elem;

  /// The maximum number of nodes per element
  unsigned int _max_nodes_per_elem;

  /// The maximum number of nodes per side
  unsigned int _max_nodes_per_side;

  /// Compute the maximum numbers per element and side
  void computeMaxPerElemAndSide();

  /// Whether this mesh is displaced
  bool _is_displaced;

  /// Build extra data for faster access to the information of extra element integers
  void buildElemIDInfo();

  /// Build lower-d mesh for all sides
  void buildLowerDMesh();

  /// Type of coordinate system per subdomain
  std::map<SubdomainID, Moose::CoordinateSystemType> & _coord_sys;

  /// Storage for RZ axis selection
  unsigned int _rz_coord_axis;

  /// Map of subdomain ID to general axisymmetric axis
  std::unordered_map<SubdomainID, std::pair<Point, RealVectorValue>> _subdomain_id_to_rz_coord_axis;

  /// A coordinate transformation object that describes how to transform this problem's coordinate
  /// system into the canonical/reference coordinate system
  std::unique_ptr<MooseAppCoordTransform> _coord_transform;

  /// Whether the coordinate system has been set
  bool _coord_system_set;

  /// Set for holding user-provided coordinate system type block names
  std::vector<SubdomainName> _provided_coord_blocks;

  /// Whether we have p-refinement (as opposed to h-refinement)
  bool _doing_p_refinement;
  /// Maximum p-refinement level of all elements
  unsigned int _max_p_level;
  /// Maximum h-refinement level of all elements
  unsigned int _max_h_level;

  template <typename T>
  struct MeshType;
};

inline MooseAppCoordTransform &
MooseMesh::coordTransform()
{
  mooseAssert(_coord_transform, "The coordinate transformation object is null.");
  return *_coord_transform;
}

template <>
struct MooseMesh::MeshType<libMesh::ReplicatedMesh>
{
  static const ParallelType value = ParallelType::REPLICATED;
};

template <>
struct MooseMesh::MeshType<libMesh::DistributedMesh>
{
  static const ParallelType value = ParallelType::DISTRIBUTED;
};

/**
 * The definition of the face_info_iterator struct.
 */
struct MooseMesh::face_info_iterator
  : variant_filter_iterator<MeshBase::Predicate, const FaceInfo *>
{
  // Templated forwarding ctor -- forwards to appropriate variant_filter_iterator ctor
  template <typename PredType, typename IterType>
  face_info_iterator(const IterType & d, const IterType & e, const PredType & p)
    : variant_filter_iterator<MeshBase::Predicate, const FaceInfo *>(d, e, p)
  {
  }
};

/**
 * The definition of the const_face_info_iterator struct. It is similar to the
 * iterator above, but also provides an additional conversion-to-const ctor.
 */
struct MooseMesh::const_face_info_iterator : variant_filter_iterator<MeshBase::Predicate,
                                                                     const FaceInfo * const,
                                                                     const FaceInfo * const &,
                                                                     const FaceInfo * const *>
{
  // Templated forwarding ctor -- forwards to appropriate variant_filter_iterator ctor
  template <typename PredType, typename IterType>
  const_face_info_iterator(const IterType & d, const IterType & e, const PredType & p)
    : variant_filter_iterator<MeshBase::Predicate,
                              const FaceInfo * const,
                              const FaceInfo * const &,
                              const FaceInfo * const *>(d, e, p)
  {
  }

  // The conversion-to-const ctor.  Takes a regular iterator and calls the appropriate
  // variant_filter_iterator copy constructor.  Note that this one is *not* templated!
  const_face_info_iterator(const MooseMesh::face_info_iterator & rhs)
    : variant_filter_iterator<MeshBase::Predicate,
                              const FaceInfo * const,
                              const FaceInfo * const &,
                              const FaceInfo * const *>(rhs)
  {
  }
};

/**
 * The definition of the elem_info_iterator struct.
 */
struct MooseMesh::elem_info_iterator
  : variant_filter_iterator<MeshBase::Predicate, const ElemInfo *>
{
  // Templated forwarding ctor -- forwards to appropriate variant_filter_iterator ctor
  template <typename PredType, typename IterType>
  elem_info_iterator(const IterType & d, const IterType & e, const PredType & p)
    : variant_filter_iterator<MeshBase::Predicate, const ElemInfo *>(d, e, p)
  {
  }
};

/**
 * The definition of the const_elem_info_iterator struct. It is similar to the
 * iterator above, but also provides an additional conversion-to-const ctor.
 */
struct MooseMesh::const_elem_info_iterator : variant_filter_iterator<MeshBase::Predicate,
                                                                     const ElemInfo * const,
                                                                     const ElemInfo * const &,
                                                                     const ElemInfo * const *>
{
  // Templated forwarding ctor -- forwards to appropriate variant_filter_iterator ctor
  template <typename PredType, typename IterType>
  const_elem_info_iterator(const IterType & d, const IterType & e, const PredType & p)
    : variant_filter_iterator<MeshBase::Predicate,
                              const ElemInfo * const,
                              const ElemInfo * const &,
                              const ElemInfo * const *>(d, e, p)
  {
  }

  // The conversion-to-const ctor.  Takes a regular iterator and calls the appropriate
  // variant_filter_iterator copy constructor.  Note that this one is *not* templated!
  const_elem_info_iterator(const MooseMesh::elem_info_iterator & rhs)
    : variant_filter_iterator<MeshBase::Predicate,
                              const ElemInfo * const,
                              const ElemInfo * const &,
                              const ElemInfo * const *>(rhs)
  {
  }
};

/**
 * The definition of the bnd_node_iterator struct.
 */
struct MooseMesh::bnd_node_iterator : variant_filter_iterator<MeshBase::Predicate, BndNode *>
{
  // Templated forwarding ctor -- forwards to appropriate variant_filter_iterator ctor
  template <typename PredType, typename IterType>
  bnd_node_iterator(const IterType & d, const IterType & e, const PredType & p)
    : variant_filter_iterator<MeshBase::Predicate, BndNode *>(d, e, p)
  {
  }
};

/**
 * The definition of the const_bnd_node_iterator struct.  It is similar to the
 * iterator above, but also provides an additional conversion-to-const ctor.
 */
struct MooseMesh::const_bnd_node_iterator : variant_filter_iterator<MeshBase::Predicate,
                                                                    BndNode * const,
                                                                    BndNode * const &,
                                                                    BndNode * const *>
{
  // Templated forwarding ctor -- forwards to appropriate variant_filter_iterator ctor
  template <typename PredType, typename IterType>
  const_bnd_node_iterator(const IterType & d, const IterType & e, const PredType & p)
    : variant_filter_iterator<MeshBase::Predicate,
                              BndNode * const,
                              BndNode * const &,
                              BndNode * const *>(d, e, p)
  {
  }

  // The conversion-to-const ctor.  Takes a regular iterator and calls the appropriate
  // variant_filter_iterator copy constructor.  Note that this one is *not* templated!
  const_bnd_node_iterator(const MooseMesh::bnd_node_iterator & rhs)
    : variant_filter_iterator<MeshBase::Predicate,
                              BndNode * const,
                              BndNode * const &,
                              BndNode * const *>(rhs)
  {
  }
};

/**
 * The definition of the bnd_elem_iterator struct.
 */
struct MooseMesh::bnd_elem_iterator : variant_filter_iterator<MeshBase::Predicate, BndElement *>
{
  // Templated forwarding ctor -- forwards to appropriate variant_filter_iterator ctor
  template <typename PredType, typename IterType>
  bnd_elem_iterator(const IterType & d, const IterType & e, const PredType & p)
    : variant_filter_iterator<MeshBase::Predicate, BndElement *>(d, e, p)
  {
  }
};

/**
 * The definition of the const_bnd_elem_iterator struct.  It is similar to the regular
 * iterator above, but also provides an additional conversion-to-const ctor.
 */
struct MooseMesh::const_bnd_elem_iterator : variant_filter_iterator<MeshBase::Predicate,
                                                                    BndElement * const,
                                                                    BndElement * const &,
                                                                    BndElement * const *>
{
  // Templated forwarding ctor -- forwards to appropriate variant_filter_iterator ctor
  template <typename PredType, typename IterType>
  const_bnd_elem_iterator(const IterType & d, const IterType & e, const PredType & p)
    : variant_filter_iterator<MeshBase::Predicate,
                              BndElement * const,
                              BndElement * const &,
                              BndElement * const *>(d, e, p)
  {
  }

  // The conversion-to-const ctor.  Takes a regular iterator and calls the appropriate
  // variant_filter_iterator copy constructor.  Note that this one is *not* templated!
  const_bnd_elem_iterator(const bnd_elem_iterator & rhs)
    : variant_filter_iterator<MeshBase::Predicate,
                              BndElement * const,
                              BndElement * const &,
                              BndElement * const *>(rhs)
  {
  }
};

/**
 * Some useful StoredRange typedefs.  These are defined *outside* the
 * MooseMesh class to mimic the Const{Node,Elem}Range classes in libmesh.
 */
typedef libMesh::StoredRange<MooseMesh::const_bnd_node_iterator, const BndNode *> ConstBndNodeRange;
typedef libMesh::StoredRange<MooseMesh::const_bnd_elem_iterator, const BndElement *>
    ConstBndElemRange;

template <typename T>
std::unique_ptr<T>
MooseMesh::buildTypedMesh(unsigned int dim)
{
  // If the requested mesh type to build doesn't match our current value for
  // _use_distributed_mesh, then we need to make sure to make our state consistent because other
  // objects, like the periodic boundary condition action, will be querying isDistributedMesh()
  if (_use_distributed_mesh != std::is_same<T, libMesh::DistributedMesh>::value)
  {
    if (getMeshPtr())
      mooseError("A MooseMesh object is being asked to build a libMesh mesh that is a different "
                 "parallel type than the libMesh mesh that it wraps. This is not allowed. Please "
                 "create another MooseMesh object to wrap the new libMesh mesh");
    setParallelType(MeshType<T>::value);
  }

  if (dim == libMesh::invalid_uint)
  {
    if (isParamValid("dim"))
      dim = getParam<MooseEnum>("dim");
    else
      // Legacy selection of the default for the 'dim' parameter
      dim = 1;
  }

  auto mesh = std::make_unique<T>(_communicator, dim);

  if (!getParam<bool>("allow_renumbering"))
    mesh->allow_renumbering(false);

  mesh->allow_remote_element_removal(_allow_remote_element_removal);
  _app.attachRelationshipManagers(*mesh, *this);

  if (_custom_partitioner_requested)
  {
    // Check of partitioner is supplied (not allowed if custom partitioner is used)
    if (!parameters().isParamSetByAddParam("partitioner"))
      mooseError("If partitioner block is provided, partitioner keyword cannot be used!");
    // Set custom partitioner
    if (!_custom_partitioner.get())
      mooseError("Custom partitioner requested but not set!");
    mesh->partitioner() = _custom_partitioner->clone();
  }
  else
    setPartitionerHelper(mesh.get());

  return mesh;
}

inline bool
MooseMesh::skipDeletionRepartitionAfterRefine() const
{
  return _skip_deletion_repartition_after_refine;
}

inline void
MooseMesh::setParallelType(ParallelType parallel_type)
{
  _parallel_type = parallel_type;
  determineUseDistributedMesh();
}

inline bool
MooseMesh::hasElementID(const std::string & id_name) const
{
  return getMesh().has_elem_integer(id_name);
}

inline unsigned int
MooseMesh::getElementIDIndex(const std::string & id_name) const
{
  if (!hasElementID(id_name))
    mooseError("Mesh does not have element ID for ", id_name);
  return getMesh().get_elem_integer_index(id_name);
}

inline bool
MooseMesh::areElemIDsIdentical(const std::string & id_name1, const std::string & id_name2) const
{
  auto id1 = getElementIDIndex(id_name1);
  auto id2 = getElementIDIndex(id_name2);
  return _id_identical_flag[id1][id2];
}

inline const std::vector<const FaceInfo *> &
MooseMesh::faceInfo() const
{
  return _face_info;
}

inline const std::vector<FaceInfo> &
MooseMesh::allFaceInfo() const
{
  return _all_face_info;
}

inline const std::map<boundary_id_type, std::vector<dof_id_type>> &
MooseMesh::nodeSetNodes() const
{
  return _node_set_nodes;
}

inline const std::unordered_map<std::pair<const Elem *, unsigned short int>, const Elem *> &
MooseMesh::getLowerDElemMap() const
{
  return _higher_d_elem_side_to_lower_d_elem;
}

inline bool
MooseMesh::isLowerD(const SubdomainID subdomain_id) const
{
  return libmesh_map_find(_sub_to_data, subdomain_id).is_lower_d;
}
