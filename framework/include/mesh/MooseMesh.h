//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "BndNode.h"
#include "BndElement.h"
#include "Restartable.h"
#include "MooseEnum.h"
#include "PerfGraphInterface.h"
#include "MooseHashing.h"
#include "MooseApp.h"
#include "FaceInfo.h"

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

// forward declaration
class MooseMesh;
class Assembly;
class RelationshipManager;
class MooseVariableBase;

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
typedef StoredRange<std::set<Node *>::iterator, Node *> SemiLocalNodeRange;

template <>
InputParameters validParams<MooseMesh>();

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
   * Returns MeshBase::mesh_dimsension(), (not
   * MeshBase::spatial_dimension()!) of the underlying libMesh mesh
   * object.
   */
  virtual unsigned int dimension() const;

  /**
   * Returns the effective spatial dimension determined by the coordinates actually used by the
   * mesh. This means that a 1D mesh that has non-zero z or y coordinates is actually a 2D or 3D
   * mesh, respectively. Likewise a 2D mesh that has non-zero z coordinates is actually 3D mesh.
   */
  virtual unsigned int effectiveSpatialDimension() const;

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
   * Calls BoundaryInfo::build_side_list().
   * Fills in the three passed vectors with list logical (element, side, id) tuples.
   * This function will eventually be deprecated in favor of the one below, which
   * returns a single std::vector of (elem-id, side-id, bc-id) tuples instead.
   */
  void buildSideList(std::vector<dof_id_type> & el,
                     std::vector<unsigned short int> & sl,
                     std::vector<boundary_id_type> & il);
  /**
   * As above, but uses the non-deprecated std::tuple interface.
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
  MeshBase::const_node_iterator localNodesBegin();
  MeshBase::const_node_iterator localNodesEnd();

  /**
   * Calls active_local_nodes_begin/end() on the underlying libMesh mesh object.
   */
  MeshBase::const_element_iterator activeLocalElementsBegin();
  const MeshBase::const_element_iterator activeLocalElementsEnd();

  /**
   * Calls n_nodes/elem() on the underlying libMesh mesh object.
   */
  virtual dof_id_type nNodes() const;
  virtual dof_id_type nElem() const;

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
  ConstElemRange * getActiveLocalElementRange();
  NodeRange * getActiveNodeRange();
  SemiLocalNodeRange * getActiveSemiLocalNodeRange() const;
  ConstNodeRange * getLocalNodeRange();
  StoredRange<MooseMesh::const_bnd_node_iterator, const BndNode *> * getBoundaryNodeRange();
  StoredRange<MooseMesh::const_bnd_elem_iterator, const BndElement *> * getBoundaryElementRange();
  ///@}

  /**
   * Returns a map of boundaries to elements.
   */
  const std::unordered_map<boundary_id_type, std::unordered_set<dof_id_type>> &
  getBoundariesToElems() const;

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
   * various boundary information on parallel meshes. Also calls update() internally. We maintain
   * the boolean parameter in order to maintain backwards compatability but it doesn't do anything
   */
  void prepare(bool = false);

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
  bool skipDeletionRepartitionAfterRefine() const
  {
    return _skip_deletion_repartition_after_refine;
  }

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
  unsigned int getGhostingPatchSize() const { return _ghosting_patch_size; };

  /**
   * Getter for the maximum leaf size parameter.
   */
  unsigned int getMaxLeafSize() const { return _max_leaf_size; };
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
  BoundingBox getInflatedProcessorBoundingBox(Real inflation_multiplier = 0.01) const;

  /**
   * Implicit conversion operator from MooseMesh -> libMesh::MeshBase.
   */
  operator libMesh::MeshBase &();
  operator const libMesh::MeshBase &() const;

  /**
   * Accessor for the underlying libMesh Mesh object.
   */
  MeshBase & getMesh();
  const MeshBase & getMesh() const;
  const MeshBase * getMeshPtr() const;

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
   * @return param boundary_name The name of the boundary.
   * @return the boundary id from the passed boundary name.
   */
  BoundaryID getBoundaryID(const BoundaryName & boundary_name) const;

  /**
   * Get the associated BoundaryID for the boundary names that are passed in.
   *
   * @return param boundary_name The names of the boundaries.
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
   * @param subdomain_name The names of the subdomains
   * @return The subdomain ids from the passed subdomain name.
   */
  std::vector<SubdomainID> getSubdomainIDs(const std::vector<SubdomainName> & subdomain_name) const;

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
  const std::string & getSubdomainName(SubdomainID subdomain_id);

  /**
   * This method sets the boundary name of the boundary based on the id parameter
   */
  void setBoundaryName(BoundaryID boundary_id, BoundaryName name);

  /**
   * Return the name of the boundary given the id.
   */
  const std::string & getBoundaryName(BoundaryID boundary_id);

  /**
   * This routine builds a multimap of boundary ids to matching boundary ids across all periodic
   * boundaries
   * in the system.
   */
  void buildPeriodicNodeMap(std::multimap<dof_id_type, dof_id_type> & periodic_node_map,
                            unsigned int var_number,
                            PeriodicBoundaries * pbs) const;

  /**
   * This routine builds a datastructure of node ids organized by periodic boundary ids
   */
  void buildPeriodicNodeSets(std::map<BoundaryID, std::set<dof_id_type>> & periodic_node_sets,
                             unsigned int var_number,
                             PeriodicBoundaries * pbs) const;

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
  bool isDistributedMesh() const { return _use_distributed_mesh; }

  /**
   * Tell the user if the distribution was overriden for any reason
   */
  bool isParallelTypeForced() const { return _parallel_type_overridden; }

  /**
   *  Allow to change parallel type
   */
  void setParallelType(ParallelType parallel_type)
  {
    _parallel_type = parallel_type;
    determineUseDistributedMesh();
  }

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
  void setCustomPartitioner(Partitioner * partitioner);

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
  virtual std::unique_ptr<PointLocatorBase> getPointLocator() const;

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
  bool hasElementID(const std::string & id_name) const
  {
    return getMesh().has_elem_integer(id_name);
  }

  /**
   * Return the accessing integer for an extra element integer with its name
   */
  unsigned int getElementIDIndex(const std::string & id_name) const
  {
    if (!hasElementID(id_name))
      mooseError("Mesh does not have element ID for ", id_name);
    return getMesh().get_elem_integer_index(id_name);
  }

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
  bool areElemIDsIdentical(const std::string & id_name1, const std::string & id_name2) const
  {
    auto id1 = getElementIDIndex(id_name1);
    auto id2 = getElementIDIndex(id_name2);
    return _id_identical_flag[id1][id2];
  }

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

  ///@{ accessors for the FaceInfo objects
  unsigned int nFace() const { return _face_info.size(); }
  /// Accessor for local \p FaceInfo objects.
  const std::vector<const FaceInfo *> & faceInfo() const
  {
    buildFaceInfo();
    return _face_info;
  }
  /// Accessor for the local FaceInfo object on the side of one element. Returns null if ghosted.
  const FaceInfo * faceInfo(const Elem * elem, unsigned int side) const;
  /// Accessor for all \p FaceInfo objects.
  const std::vector<FaceInfo> & allFaceInfo() const
  {
    buildFaceInfo();
    return _all_face_info;
  }
  ///@}

  /**
   * Cache \p elem and \p neighbor dof indices information for variables in all the local \p
   * FaceInfo objects to save computational expense
   */
  void cacheVarIndicesByFace(const std::vector<const MooseVariableBase *> & moose_vars);

  /**
   * Compute the face coordinate value for all \p FaceInfo objects. 'Coordinate' here means a
   * coordinate value associated with the coordinate system. For Cartesian coordinate systems,
   * 'coordinate' is simply '1'; in RZ, '2*pi*r', and in spherical, '4*pi*r^2'
   */
  void computeFaceInfoFaceCoords();

  /**
   * Set whether this mesh is displaced
   */
  void isDisplaced(bool is_displaced) { _is_displaced = is_displaced; }

  /**
   * whether this mesh is displaced
   */
  bool isDisplaced() const { return _is_displaced; }

  /**
   * @return A map from nodeset ids to the vector of node ids in the nodeset
   */
  const std::map<boundary_id_type, std::vector<dof_id_type>> & nodeSetNodes() const
  {
    return _node_set_nodes;
  }

  /**
   * Get the coordinate system type, e.g. xyz, rz, or r-spherical, for the provided subdomain id \p
   * sid
   */
  Moose::CoordinateSystemType getCoordSystem(SubdomainID sid) const;

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
   * Mark the face information as dirty
   */
  void faceInfoDirty() { _face_info_dirty = true; }

protected:
  /// Deprecated (DO NOT USE)
  std::vector<std::unique_ptr<GhostingFunctor>> _ghosting_functors;

  /// The list of active geometric relationship managers (bound to the underlying MeshBase object).
  std::vector<std::shared_ptr<RelationshipManager>> _relationship_managers;

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

  /// The partitioner used on this mesh
  MooseEnum _partitioner_name;
  bool _partitioner_overridden;

  /// The custom partitioner
  std::unique_ptr<Partitioner> _custom_partitioner;
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
  std::unique_ptr<ConstElemRange> _active_local_elem_range;

  std::unique_ptr<SemiLocalNodeRange> _active_semilocal_node_range;
  std::unique_ptr<NodeRange> _active_node_range;
  std::unique_ptr<ConstNodeRange> _local_node_range;
  std::unique_ptr<StoredRange<MooseMesh::const_bnd_node_iterator, const BndNode *>> _bnd_node_range;
  std::unique_ptr<StoredRange<MooseMesh::const_bnd_elem_iterator, const BndElement *>>
      _bnd_elem_range;

  /// A map of all of the current nodes to the elements that they are connected to.
  std::map<dof_id_type, std::vector<dof_id_type>> _node_to_elem_map;
  bool _node_to_elem_map_built;

  /// A map of all of the current nodes to the active elements that they are connected to.
  std::map<dof_id_type, std::vector<dof_id_type>> _node_to_active_semilocal_elem_map;
  bool _node_to_active_semilocal_elem_map_built;

  /**
   * A set of subdomain IDs currently present in the mesh. For parallel meshes, includes subdomains
   * defined on other processors as well.
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

  void cacheInfo();
  void freeBndNodes();
  void freeBndElems();
  void setPartitionerHelper(MeshBase * mesh = nullptr);

private:
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
  mutable bool _face_info_dirty = true;

  /**
   * Builds the face info vector that stores meta-data needed for looping over and doing
   * calculations based on mesh faces. We build face information only upon request and only if the
   * \p _face_info_dirty flag is false, either because this method has yet to be called or
   * because someone called \p update() indicating the mesh has changed. Face information is only
   * requested by getters that should appear semantically const. Consequently this method must
   * also be marked const and so we make it and all associated face information data private to
   * prevent misuse
   */
  void buildFaceInfo() const;

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
   * to copy from and to for stateful material properties on newly created elements from Adaptivity.
   *
   * @param elem The element that represents the element type you need the refinement map for.
   * @param qrule The quadrature rule in use.
   * @param qrule_face The current face quadrature rule
   * @param parent_side The side of the parent to map (-1 if not mapping parent sides)
   * @param child The child number (-1 if not mapping child internal sides)
   * @param child_side The side number of the child (-1 if not mapping sides)
   */
  void buildRefinementMap(const Elem & elem,
                          QBase & qrule,
                          QBase & qrule_face,
                          int parent_side,
                          int child,
                          int child_side);

  /**
   * Build the coarsening map for a given element type.  This will tell you what quadrature points
   * to copy from and to for stateful material properties on newly created elements from Adaptivity.
   *
   * @param elem The element that represents the element type you need the coarsening map for.
   * @param qrule The quadrature rule in use.
   * @param qrule_face The current face quadrature rule
   * @param input_side The side to map
   */
  void buildCoarseningMap(const Elem & elem, QBase & qrule, QBase & qrule_face, int input_side);

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
   * Given an elem type, get maps that tell us what qp's are closest to each other between a parent
   * and it's children.
   * This is mainly used for mapping stateful material properties during adaptivity.
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
                            QBase & qrule,
                            QBase & qrule_face,
                            std::vector<std::vector<QpMap>> & refinement_map,
                            std::vector<std::pair<unsigned int, QpMap>> & coarsen_map,
                            int parent_side,
                            int child,
                            int child_side);

  /// Holds mappings for volume to volume and parent side to child side
  std::map<std::pair<int, ElemType>, std::vector<std::vector<QpMap>>> _elem_type_to_refinement_map;

  /// Holds mappings for "internal" child sides to parent volume.  The second key is (child, child_side).
  std::map<ElemType, std::map<std::pair<int, int>, std::vector<std::vector<QpMap>>>>
      _elem_type_to_child_side_refinement_map;

  /// Holds mappings for volume to volume and parent side to child side
  std::map<std::pair<int, ElemType>, std::vector<std::pair<unsigned int, QpMap>>>
      _elem_type_to_coarsening_map;

  /// Holds a map from subomdain ids to the neighboring subdomain ids
  std::unordered_map<SubdomainID, std::set<SubdomainID>> _sub_to_neighbor_subs;

  /// Holds a map from subomdain ids to the boundary ids that are attached to it
  std::unordered_map<SubdomainID, std::set<BoundaryID>> _subdomain_boundary_ids;

  /// Holds a map from neighbor subomdain ids to the boundary ids that are attached to it
  std::unordered_map<SubdomainID, std::set<BoundaryID>> _neighbor_subdomain_boundary_ids;

  /// Holds a map from a high-order element side to its corresponding lower-d element
  std::unordered_map<std::pair<const Elem *, unsigned short int>, const Elem *>
      _higher_d_elem_side_to_lower_d_elem;
  std::unordered_map<const Elem *, unsigned short int> _lower_d_elem_to_higher_d_elem_side;

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
  /// when posssible since it is not scalable
  bool _need_ghost_ghosted_boundaries;

  /// Unique element integer IDs for each subdomain and each extra element integers
  std::vector<std::unordered_map<SubdomainID, std::set<dof_id_type>>> _block_id_mapping;
  /// Maximum integer ID for each extra element integer
  std::vector<dof_id_type> _max_ids;
  /// Minimum integer ID for each extra element integer
  std::vector<dof_id_type> _min_ids;
  /// Flags to indicate whether or not any two extra element integers are the same
  std::vector<std::vector<bool>> _id_identical_flag;

  /// Whether this mesh is displaced
  bool _is_displaced;

  /// Build extra data for faster access to the information of extra element integers
  void buildElemIDInfo();

  /// Build lower-d mesh for all sides
  void buildLowerDMesh();

  /// Type of coordinate system per subdomain
  std::map<SubdomainID, Moose::CoordinateSystemType> _coord_sys;

  /// Storage for RZ axis selection
  unsigned int _rz_coord_axis;

  template <typename T>
  struct MeshType;
};

template <>
struct MooseMesh::MeshType<ReplicatedMesh>
{
  static const ParallelType value = ParallelType::REPLICATED;
};

template <>
struct MooseMesh::MeshType<DistributedMesh>
{
  static const ParallelType value = ParallelType::DISTRIBUTED;
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
typedef StoredRange<MooseMesh::const_bnd_node_iterator, const BndNode *> ConstBndNodeRange;
typedef StoredRange<MooseMesh::const_bnd_elem_iterator, const BndElement *> ConstBndElemRange;

template <typename T>
std::unique_ptr<T>
MooseMesh::buildTypedMesh(unsigned int dim)
{
  // If the requested mesh type to build doesn't match our current value for _use_distributed_mesh,
  // then we need to make sure to make our state consistent because other objects, like the periodic
  // boundary condition action, will be querying isDistributedMesh()
  if (_use_distributed_mesh != std::is_same<T, DistributedMesh>::value)
    setParallelType(MeshType<T>::value);

  if (dim == libMesh::invalid_uint)
    dim = getParam<MooseEnum>("dim");

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
