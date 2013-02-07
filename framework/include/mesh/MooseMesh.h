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

#ifndef MOOSEMESH_H
#define MOOSEMESH_H

#include "InputParameters.h"
#include "MooseObject.h"
#include "BndNode.h"
#include "BndElement.h"
#include "MooseTypes.h"

// libMesh
#include "mesh.h"
#include "boundary_info.h"
#include "elem_range.h"
#include "node_range.h"
#include "periodic_boundaries.h"
#include "quadrature.h"

#include <map>

//forward declaration
class MooseMesh;
class NonlinearSystem;
class Assembly;

typedef StoredRange<std::set<Node *>::iterator, Node*> SemiLocalNodeRange;

template<>
InputParameters validParams<MooseMesh>();

/**
 * Helper object for holding qp mapping info.
 */
class QpMap
{
public:
  QpMap():_distance(std::numeric_limits<Real>::max()) {}

  /// The qp to map from
  unsigned int _from;

  /// The qp to map to
  unsigned int _to;

  /// The distance between them
  Real _distance;
};

// NOTE: maybe inheritance would be better here
//
class MooseMesh : public MooseObject
{
public:
  MooseMesh(const std::string & name, InputParameters parameters);
  MooseMesh(const MooseMesh & other_mesh);
  virtual ~MooseMesh();

  virtual unsigned int dimension() const { return _mesh.mesh_dimension(); }

  std::vector<BoundaryID> boundary_ids (const Elem *const elem, const unsigned short int side) const { return _mesh.boundary_info->boundary_ids(elem, side); }
  const std::set<BoundaryID> & get_boundary_ids () const { return _mesh.boundary_info->get_boundary_ids(); }

  void buildNodeList ();
  void buildBndElemList();
  std::map<unsigned int, std::vector<unsigned int> > & nodeToElemMap();

  virtual bnd_node_iterator bnd_nodes_begin ();
  virtual bnd_node_iterator bnd_nodes_end ();

  virtual bnd_elem_iterator bnd_elems_begin ();
  virtual bnd_elem_iterator bnd_elems_end ();

  void build_node_list_from_side_list() { _mesh.boundary_info->build_node_list_from_side_list(); }
  void build_side_list(std::vector<unsigned int> & el, std::vector<unsigned short int> & sl, std::vector<short int> & il) { _mesh.boundary_info->build_side_list(el, sl, il); }
  unsigned int side_with_boundary_id(const Elem * const elem, const BoundaryID boundary_id) const { return _mesh.boundary_info->side_with_boundary_id(elem, boundary_id); }

  MeshBase::const_node_iterator local_nodes_begin() { return _mesh.local_nodes_begin(); }
  MeshBase::const_node_iterator local_nodes_end() { return _mesh.local_nodes_end(); }

  MeshBase::const_element_iterator active_local_elements_begin() { return _mesh.active_local_elements_begin(); }
  const MeshBase::const_element_iterator active_local_elements_end() { return _mesh.active_local_elements_end(); }

  virtual unsigned int n_nodes () const { return _mesh.n_nodes(); }
  virtual unsigned int n_elem () const { return _mesh.n_elem(); }

  virtual const Node & node (const unsigned int i) const;
  virtual Node & node (const unsigned int i);
  virtual const Node* node_ptr (const unsigned int i) const;
  virtual Node* node_ptr (const unsigned int i);

  virtual Elem * elem(const unsigned int i) { return _mesh.elem(i); }
  virtual const Elem * elem(const unsigned int i) const { return _mesh.elem(i); }

  bool changed() { return _is_changed; }
  void changed(bool state) { _is_changed = state; }

  bool parallel() { return _is_parallel; }

  void meshChanged();

  /**
   * Cache information about what elements were refined and coarsened in the previous step.
   */
  void cacheChangedLists();

  /**
   * Return a range that is suitable for threaded execution over elements that were just refined.
   *
   * @return The _Parent_ elements that are now set to be INACTIVE.  Their _children_ are the new elements.
   */
  ConstElemPointerRange * refinedElementRange();

  /**
   * Return a range that is suitable for threaded execution over elements that were just coarsend.
   * Note that these are the _Parent_ elements that are now set to be INACTIVE.  Their _children_
   * are the elements that were just removed.  Use coarsenedElementChildren() to get the elment
   * IDs for the children that were just removed for a particular parent element.
   */
  ConstElemPointerRange * coarsenedElementRange();

  /**
   * Get the newly removed children element ids for an element that was just coarsened.
   *
   * @parent_id The element ID of the parent element that was coarsened to.
   * @return The child element ids in Elem::child() order.
   */
  std::vector<const Elem *> & coarsenedElementChildren(const Elem * elem);

  void updateActiveSemiLocalNodeRange(std::set<unsigned int> & ghosted_elems);

  ConstElemRange * getActiveLocalElementRange();
  NodeRange * getActiveNodeRange();
  SemiLocalNodeRange * getActiveSemiLocalNodeRange();
  ConstNodeRange * getLocalNodeRange();
  ConstBndNodeRange * getBoundaryNodeRange();
  ConstBndElemRange * getBoundaryElementRange();

  const std::set<SubdomainID> & meshSubdomains() { return _mesh_subdomains; }
  const std::set<BoundaryID> & meshBoundaryIds() { return _mesh_boundary_ids; }

  void read(const std::string file_name);

  void prepare();
  void update();

  /**
   * This will add the boundary ids to be ghosted to this processor
   */
  void addGhostedBoundary(BoundaryID boundary_id) { _ghosted_boundaries.insert(boundary_id); }

  /**
   * This sets the inflation amount for the bounding box for each partition for use in
   * ghosting boundaries
   */
  void setGhostedBoundaryInflation(const std::vector<Real> & inflation) { _ghosted_boundaries_inflation = inflation; }

  std::set<unsigned int> & getGhostedBoundaries() { return _ghosted_boundaries; }

  std::vector<Real> & getGhostedBoundaryInflation() { return _ghosted_boundaries_inflation; }

  void setPatchSize(const unsigned int patch_size) { _patch_size = patch_size; }
  unsigned int getPatchSize() { return _patch_size; }

  operator libMesh::Mesh &(void) { return _mesh; }

  MeshBase & getMesh() { return _mesh; }

  inline void printInfo(std::ostream &os=libMesh::out) { _mesh.print_info(os); }

  std::set<SubdomainID> & getNodeBlockIds(const Node & node);

  std::vector<unsigned int> & getNodeList(short int nodeset_id) { return _node_set_nodes[nodeset_id]; }

  // Get/Set Filename (for meshes read from a file)
  void setFileName(const std::string & file_name) { _file_name = file_name; }
  const std::string & getFileName() const { return _file_name; }

  /**
   * Add a new node to the mesh.  If there is already a node located at the point passed
   * then the node will not be added.  In either case a refernce to the node at that location
   * will be returned
   */
  const Node * addUniqueNode(const Point & p, Real tol=1e-6);

  /**
   * Adds a ficticious "QuadratureNode".  This doesn't actually add it to the libMesh mesh...
   * we just keep track of these here in MooseMesh.
   *
   * QuadratureNodes are fictitious "Nodes" that are located at quadrature points.  This is useful for using
   * the geometric search system to do searches based on quadrature point locations....
   *
   * @param elem The element
   */
  Node * addQuadratureNode(const Elem * elem, const unsigned short int side, const unsigned int qp, BoundaryID bid, const Point & point);

  /**
   * Get a specified quadrature node.
   *
   * @param elem The element the quadrature point is on
   * @param side The side the quadrature point is on
   * @param qp The quadrature point number associated with the point
   */
  Node * getQuadratureNode(const Elem * elem, const unsigned short int side, const unsigned int qp);

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
  std::vector<BoundaryID> getBoundaryIDs(const std::vector<BoundaryName> & boundary_name) const;

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
   * This method returns a writable reference to a subdomain name based on the id parameter
   */
  void setSubdomainName(SubdomainID subdomain_id, SubdomainName name);

  /**
   * This method returns a writable reference to a boundary name based on the id parameter
   */
  void setBoundaryName(BoundaryID boundary_id, BoundaryName name);

  libMesh::Mesh _mesh;

  /**
   * This routine builds a multimap of boundary ids to matching boundary ids across all periodic boundaries
   * in the system.  It does this only for active local elements so the list will not be globally complete when
   * run in parallel!
   */
  void buildPeriodicNodeMap(std::multimap<unsigned int, unsigned int> & periodic_node_map, unsigned int var_number, PeriodicBoundaries *pbs) const;

  /**
   * This routine builds a datastructure of node ids organized by periodic boundary ids
   */
  void buildPeriodicNodeSets(std::map<BoundaryID, std::set<unsigned int> > & periodic_node_sets, unsigned int var_number, PeriodicBoundaries *pbs) const;

  /**
   * Returns the width of the requested dimension
   */
  Real dimensionWidth(unsigned int component) const;

  /**
   * Returns the min or max of the requested dimension respectively
   */
  Real getMinInDimension(unsigned int component) const;
  Real getMaxInDimension(unsigned int component) const;

  void addPeriodicVariable(unsigned int var_num, BoundaryID primary, BoundaryID secondary);

  /**
   * Returns whether this generated mesh is periodic in the given dimension
   * for the given variable.
   * @param nonlinear_var_num - The nonlinear variable number
   * @param component - An integer representing the desired component (dimension)
   */
  bool isTranslatedPeriodic(unsigned int nonlinear_var_num, unsigned int component) const;

  /**
   * This function returns the distance between two points on the mesh taking into account periodicity
   * for the given variable number.
   * @param nonlinear_var_num - The nonlinear variable number
   * @param p, q - The points for which to compute a minimum distance
   * @return Real - The L2 distance between p and q
   */
  Real minPeriodicDistance(unsigned int nonlinear_var_num, Point p, Point q) const;

  /**
   * This function returns the paired boundary ids for the given component.  For example, in a generated
   * 2D mesh, passing 0 for the "x" component will return (3, 1).
   * @param component - An integer representing the desired component (dimension)
   * @return std::pair - The matching boundary pairs for the passed component
   */
  std::pair<BoundaryID, BoundaryID> getPairedBoundaryMapping(unsigned int component);

  /**
   * Create the refinement and coarsening maps necessary for projection of stateful material properties
   * when using adaptivity.
   *
   * @param qrule A representative volume qrule
   * @param qrule_face A representative face qrule
   */
  void buildRefinementAndCoarseningMaps(Assembly * assembly);

  /**
   * Get the refinement map for a given element type.  This will tell you what quadrature points
   * to copy from and to for stateful material properties on newly created elements from Adaptivity.
   *
   * @param elem The element that represents the element type you need the refinement map for.
   * @param qrule The quadrature rule in use.
   * @param qrule_face The current face quadrature rule
   * @param parent_side The side of the parent to map (-1 if not mapping parent sides)
   * @param child The child number (-1 if not mapping child internal sides)
   * @param child_side The side number of the child (-1 if not mapping sides)
   */
  const std::vector<std::vector<QpMap> > & getRefinementMap(const Elem & elem, int parent_side, int child, int child_side);

  /**
   * Get the coarsening map for a given element type.  This will tell you what quadrature points
   * to copy from and to for stateful material properties on newly created elements from Adaptivity.
   *
   * @param elem The element that represents the element type you need the coarsening map for.
   * @param qrule The quadrature rule in use.
   * @param qrule_face The current face quadrature rule
   * @param input_side The side to map
   */
  const std::vector<std::pair<unsigned int, QpMap> > & getCoarseningMap(const Elem & elem, int input_side);

  void changeBoundaryId(const boundary_id_type old_id, const boundary_id_type new_id, bool delete_prev);

protected:
  /// Convienence enums
  enum {X=0, Y, Z};
  enum {MIN=0, MAX};

  /// true if mesh is changed (i.e. after adaptivity step)
  bool _is_changed;

  /// True if using a TRUE parallel mesh (ie Nemesis)
  bool _is_parallel;

  /// The elements that were just refined.
  ConstElemPointerRange * _refined_elements;

  /// The elements that were just coarsened.
  ConstElemPointerRange * _coarsened_elements;

  /// Map of Parent elements to child elements for elements that were just coarsened.  NOTE: the child element pointers ARE PROBABLY INVALID.  Only use them for indexing!
  std::map<const Elem *, std::vector<const Elem *> > _coarsened_element_children;

  /// Used for generating the semilocal node range
  std::set<Node *> _semilocal_node_list;

  /**
   * A range for use with TBB.  We do this so that it doesn't have
   * to get rebuilt all the time (which takes time).
   */
  ConstElemRange * _active_local_elem_range;
  SemiLocalNodeRange * _active_semilocal_node_range;  ///< active local + active ghosted
  NodeRange * _active_node_range;
  ConstNodeRange * _local_node_range;
  ConstBndNodeRange * _bnd_node_range;
  ConstBndElemRange * _bnd_elem_range;

  /// A map of all of the current nodes to the elements that they are connected to.
  std::map<unsigned int, std::vector<unsigned int> > _node_to_elem_map;
  bool _node_to_elem_map_built;

  /**
   * A set of subdomain IDs currently present in the mesh.
   * For parallel meshes, includes subdomains defined on other
   * processors as well.
   */
  std::set<SubdomainID> _mesh_subdomains;

  /**
   * A set of boundary IDs currently present in the mesh.
   * In serial, this is equivalent to the values returned
   * by _mesh.boundary_info->get_boundary_ids().  In parallel,
   * it will contain off-processor boundary IDs as well.
   */
  std::set<BoundaryID> _mesh_boundary_ids;

  /// array of boundary nodes
  std::vector<BndNode *> _bnd_nodes;
  typedef std::vector<BndNode *>::iterator             bnd_node_iterator_imp;
  typedef std::vector<BndNode *>::const_iterator const_bnd_node_iterator_imp;

  /// array of boundary elems
  std::vector<BndElement *> _bnd_elems;
  typedef std::vector<BndElement *>::iterator             bnd_elem_iterator_imp;
  typedef std::vector<BndElement *>::const_iterator const_bnd_elem_iterator_imp;

  std::map<unsigned int, Node *> _quadrature_nodes;
  std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, Node *> > > _elem_to_side_to_qp_to_quadrature_nodes;
  std::vector<BndNode> _extra_bnd_nodes;

  /// list of nodes that belongs to a specified block (domain)
  std::map<unsigned int, std::set<SubdomainID> > _block_node_list;

  /// list of nodes that belongs to a specified nodeset: indexing [nodeset_id] -> <array of node ids>
  std::map<short int, std::vector<unsigned int> > _node_set_nodes;

  std::set<unsigned int> _ghosted_boundaries;
  std::vector<Real> _ghosted_boundaries_inflation;

  /// The number of nodes to consider in the NearestNode neighborhood.
  unsigned int _patch_size;

  /// file_name iff this mesh was read from a file
  std::string _file_name;

  /// Vector of all the Nodes in the mesh for determining when to add a new point
  std::vector<Node *> _node_map;

  /// Boolean indiciating whether this mesh was detected to be regular and orthogonal
  bool _regular_orthogonal_mesh;

  /// The bounds in each dimension of the mesh for regular orthogonal meshes
  std::vector<std::vector<Real> > _bounds;

  /// A vector holding the paired boundaries for a regular orthogonal mesh
  std::vector<std::pair<BoundaryID, BoundaryID> > _paired_boundary;

  void cacheInfo();
  void freeBndNodes();
  void freeBndElems();

private:
  /**
   * A map of vectors indicating which dimensions are periodic in a regular orthogonal mesh for
   * the specified variable numbers.  This data structure is populated by addPeriodicVariable.
   */
  std::map<unsigned int, std::vector<bool> > _periodic_dim;

  /**
   * A convience vector used to hold values in each dimension representing half of the range.
   */
  RealVectorValue _half_range;

  /// A vector containing the nodes at the corners of a regular orthogonal mesh
  std::vector<Node *> _extreme_nodes;

  /**
   * This routine determines whether the Mesh is a regular orthogonal mesh (i.e. square in 2D, cubic in 3D).
   * If it is, then we can use a number of convience functions when periodic boundary conditions
   * are applied.  This routine populates the _range vector which is necessary for these convenience functions.
   * Note:  This routine can potentially identify meshes with concave faces that still "fit" in the convex hull
   * of the corresponding regular orthogonal mesh.  This case is highly unlikely in practice and if a user
   * does this, well.... release the kicker!
   */
  bool detectOrthogonalDimRanges(Real tol=1e-6);

  /**
   * This routine detects paired sidesets of a regular orthogonal mesh (.i.e. parallel sidesets "across" from one and other).
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
  void buildRefinementMap(const Elem & elem, QBase & qrule, QBase & qrule_face, int parent_side, int child, int child_side);

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
   * @param from The reference positions in the parent of the the points we're mapping _to_
   * @param qp_map This will be filled with QpMap objects holding the mappings.
   */
  void mapPoints(const std::vector<Point> & from, const std::vector<Point> & to, std::vector<QpMap> & qp_map);

  /**
   * Given an elem type, get maps that tell us what qp's are closest to eachother between a parent and it's children.
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
   * @param refinement_map The map to use when an element gets split
   * @param coarsen_map The map to use when an element is coarsened.
   */
  void findAdaptivityQpMaps(const Elem * template_elem,
                            QBase & qrule,
                            QBase & qrule_face,
                            std::vector<std::vector<QpMap> > & refinement_map,
                            std::vector<std::pair<unsigned int, QpMap> > & coarsen_map,
                            int parent_side,
                            int child,
                            int child_side);

  /// Holds mappings for volume to volume and parent side to child side
  std::map<std::pair<int, ElemType>, std::vector<std::vector<QpMap> > > _elem_type_to_refinement_map;

  /// Holds mappings for "internal" child sides to parent volume.  The second key is (child, child_side).
  std::map<ElemType, std::map<std::pair<int, int>, std::vector<std::vector<QpMap> > > > _elem_type_to_child_side_refinement_map;

  /// Holds mappings for volume to volume and parent side to child side
  std::map<std::pair<int, ElemType>, std::vector<std::pair<unsigned int, QpMap> > > _elem_type_to_coarsening_map;
};

#endif /* MOOSEMESH_H */
