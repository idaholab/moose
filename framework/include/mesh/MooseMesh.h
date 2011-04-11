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
#include "BndNode.h"

// libMesh
#include "mesh.h"
#include "boundary_info.h"
#include "elem_range.h"
#include "node_range.h"

//forward declaration
class MeshModifier;

typedef std::set<subdomain_id_type>::const_iterator SubdomainIterator;

// NOTE: maybe inheritance would be better here
//
class MooseMesh
{
public:
  MooseMesh(int mesh_dim = 1);
  MooseMesh(const MooseMesh & other_mesh);
  virtual ~MooseMesh();

  unsigned int dimension() { return _mesh.mesh_dimension(); }

  std::vector<short int> boundary_ids (const Elem *const elem, const unsigned short int side) const { return _mesh.boundary_info->boundary_ids(elem, side); }
  const std::set<short int> & get_boundary_ids () const { return _mesh.boundary_info->get_boundary_ids(); }

  void buildNodeList ();

  virtual bnd_node_iterator bnd_nodes_begin ();
  virtual bnd_node_iterator bnd_nodes_end ();

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
  ConstBndNodeRange * getBoundaryNodeRange();

  const std::set<subdomain_id_type> & meshSubdomains() { return _mesh_subdomains; }

  void read(const std::string file_name);

  void prepare();

  void uniformlyRefine(int level);

  operator libMesh::Mesh &(void) { return _mesh; }

  // Mesh Modifiers /////
  void addMeshModifer(const std::string & mod_name, const std::string & name, InputParameters parameters);
  void applyMeshModifications();

  inline void printInfo(std::ostream &os=libMesh::out) { _mesh.print_info(os); }
  
  std::set<subdomain_id_type> & getNodeBlockIds(const Node & node);

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
  ConstBndNodeRange * _bnd_node_range;

  /**
   * A map of all of the current nodes to the elements that they are connected to.
   */
  std::vector<std::vector<unsigned int> > _node_to_elem_map;

  std::set<subdomain_id_type> _mesh_subdomains;

  std::vector<MeshModifier *> _mesh_modifiers;

  std::vector<BndNode *> _bnd_nodes;                                        /// array of boundary nodes
  typedef std::vector<BndNode *>::iterator             bnd_node_iterator_imp;
  typedef std::vector<BndNode *>::const_iterator const_bnd_node_iterator_imp;

  std::map<unsigned int, std::set<subdomain_id_type> > _block_node_list;        /// list of nodes that belongs to a specified block (domain)

  void cacheInfo();
  void freeBndNodes();
};

#endif /* MOOSEMESH_H */
