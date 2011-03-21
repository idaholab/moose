#ifndef MESH_H_
#define MESH_H_

#include "InputParameters.h"

// libMesh
#include "mesh.h"
#include "boundary_info.h"
#include "elem_range.h"
#include "node_range.h"

//forward declaration
class MeshModifier;


namespace Moose
{

typedef StoredRange<MeshBase::node_iterator,             Node*>      NodeRange;
typedef StoredRange<MeshBase::const_node_iterator, const Node*> ConstNodeRange;

// NOTE: maybe inheritance would be better here
//
class Mesh
{
public:
  Mesh(int mesh_dim);
  Mesh(const Mesh & other_mesh);
  virtual ~Mesh();

  unsigned int dimension() { return _mesh.mesh_dimension(); }

  std::vector<short int> boundary_ids (const Elem *const elem, const unsigned short int side) const { return _mesh.boundary_info->boundary_ids(elem, side); }

  void buildBoudndaryNodeList ();
  void build_node_list (std::vector< unsigned int > &nl, std::vector< short int > &il) const { _mesh.boundary_info->build_node_list(nl, il); }
  void build_node_list_from_side_list() { _mesh.boundary_info->build_node_list_from_side_list(); }
  void build_side_list(std::vector<unsigned int> & el, std::vector<unsigned short int> & sl, std::vector<short int> & il) { _mesh.boundary_info->build_side_list(el, sl, il); }
  unsigned int side_with_boundary_id(const Elem * const elem, const unsigned short int boundary_id) const { return _mesh.boundary_info->side_with_boundary_id(elem, boundary_id); }

  MeshBase::const_node_iterator local_nodes_begin() { return _mesh.local_nodes_begin(); }
  MeshBase::const_node_iterator local_nodes_end() { return _mesh.local_nodes_end(); }

  MeshBase::const_element_iterator active_local_elements_begin() { return _mesh.active_local_elements_begin(); }
  const MeshBase::const_element_iterator active_local_elements_end() { return _mesh.active_local_elements_end(); }

  virtual unsigned int n_nodes () const { return _mesh.n_nodes(); }
  virtual unsigned int n_elem () const { return _mesh.n_elem(); }

  std::vector<std::vector<unsigned int> > & nodeToElemMap() { return _node_to_elem_map; }

  virtual const Node & node (const unsigned int i) const { return _mesh.node(i); }
  virtual Node & node (const unsigned int i) { return _mesh.node(i); }

  virtual Elem * elem(const unsigned int i) const { return _mesh.elem(i); }

  bool changed() { return _is_changed; }

  void changed(bool state) { _is_changed = state; }

  void meshChanged();

  ConstElemRange * getActiveLocalElementRange();
  NodeRange * getActiveNodeRange();
  ConstNodeRange * getLocalNodeRange();
  ConstNodeRange * getBoundaryNodeRange();

  void read(const std::string file_name);

  void prepare();

  void uniformlyRefine(int level);

  operator libMesh::Mesh &(void) { return _mesh; }

  // Mesh Modifiers /////
  void addMeshModifer(const std::string & mod_name, const std::string & name, InputParameters parameters);
  void applyMeshModifications();


public:
  libMesh::Mesh _mesh;

protected:
  bool _is_changed;                   /// true if mesh is changed (i.e. after adaptivity step)

  /**
   * A range for use with TBB.  We do this so that it doesn't have
   * to get rebuilt all the time (which takes time).
   */
  ConstElemRange * _active_local_elem_range;
  NodeRange * _active_node_range;
  ConstNodeRange * _local_node_range;
  ConstNodeRange * _bnd_node_range;

  std::vector<unsigned int> _bnd_nodes;
  std::vector<short int> _bnd_ids;

  /**
   * A map of all of the current nodes to the elements that they are connected to.
   */
  std::vector<std::vector<unsigned int> > _node_to_elem_map;

  std::vector<MeshModifier *> _mesh_modifiers;
};

} // namespace

#endif /* MESH_H_ */
