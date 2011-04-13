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

#include "MooseMesh.h"
#include "Factory.h"
#include "MeshModifier.h"

// libMesh
#include "boundary_info.h"
#include "mesh_tools.h"
#include "mesh_refinement.h"

static const int GRAIN_SIZE = 1;     // the grain_size does not have much influence on our execution speed


MooseMesh::MooseMesh(int mesh_dim) :
    _mesh(mesh_dim),
    _is_changed(false),
    _init_refinement_level(0),
    _active_local_elem_range(NULL),
    _active_semilocal_node_range(NULL),
    _active_node_range(NULL),
    _local_node_range(NULL),
    _bnd_node_range(NULL)
{
}

MooseMesh::MooseMesh(const MooseMesh & other_mesh) :
    _mesh(other_mesh._mesh),
    _is_changed(false),
    _init_refinement_level(other_mesh._init_refinement_level),
    _active_local_elem_range(NULL),
    _active_semilocal_node_range(NULL),
    _active_node_range(NULL),
    _local_node_range(NULL),
    _bnd_node_range(NULL)
{
  (*_mesh.boundary_info) = (*other_mesh._mesh.boundary_info);
}

MooseMesh::~MooseMesh()
{
  freeBndNodes();

  delete _active_local_elem_range;
  delete _active_node_range;
  delete _active_semilocal_node_range;
  delete _local_node_range;
  delete _bnd_node_range;
}

void
MooseMesh::freeBndNodes()
{
  // free memory
  for (std::vector<BndNode *>::iterator it = _bnd_nodes.begin(); it != _bnd_nodes.end(); ++it)
    delete (*it);
}

void
MooseMesh::read(const std::string file_name)
{
  _mesh.read(file_name);
}

void
MooseMesh::prepare()
{
  _mesh.prepare_for_use(false);

  // If using ParallelMesh this will delete non-local elements from the current processor
  _mesh.delete_remote_elements();

  const MeshBase::element_iterator el_end = _mesh.elements_end();
  for (MeshBase::element_iterator el = _mesh.elements_begin(); el != el_end; ++el)
    _mesh_subdomains.insert((*el)->subdomain_id());

  update();
}

void
MooseMesh::update()
{
  // Rebuild the boundary conditions
  build_node_list_from_side_list();
  //Update the node to elem map
  MeshTools::build_nodes_to_elem_map(_mesh, _node_to_elem_map);
  buildNodeList();
  cacheInfo();
}

void
MooseMesh::uniformlyRefine(int level)
{
  MeshRefinement mesh_refinement(_mesh);
  if (level)
    mesh_refinement.uniformly_refine(level);
  else if (_init_refinement_level)
    mesh_refinement.uniformly_refine(_init_refinement_level);
}

void
MooseMesh::meshChanged()
{
  update();

  // Rebuild the active local element range
  delete _active_local_elem_range;
  _active_local_elem_range = NULL;
  // Rebuild the node range
  delete _active_node_range;
  _active_node_range = NULL;
  // Rebuild the semilocal range
  delete _active_semilocal_node_range;
  _active_semilocal_node_range = NULL;
  // Rebuild the local node range
  delete _local_node_range;
  _local_node_range = NULL;
  // Rebuild the boundary node range
  delete _bnd_node_range;
  _bnd_node_range = NULL;

  // Rebuild the ranges
  getActiveLocalElementRange();
  getActiveNodeRange();
  getLocalNodeRange();
  getBoundaryNodeRange();

  // Print out information about the adapted mesh if requested
//  if (_print_mesh_changed)
//  {
//    std::cout << "\nMesh Changed:\n";
//    _mesh->print_info();
//  }

  // Lets the output system know that the mesh has changed recently.
  _is_changed = true;
}

void
MooseMesh::updateActiveSemiLocalNodeRange(std::set<unsigned int> & ghosted_elems)
{
  _semilocal_node_list.clear();
    
  ConstNodeRange * local_nodes = getLocalNodeRange();
  
  // First add the local nodes
  for(ConstNodeRange::const_iterator it=local_nodes->begin();
      it != local_nodes->end();
      ++it)
    _semilocal_node_list.insert(const_cast<Node *>(*it));

  // Now add the nodes connected to ghosted_elems
  for(std::set<unsigned int>::iterator it=ghosted_elems.begin();
      it!=ghosted_elems.end();
      ++it)
  {
    Elem * elem = _mesh.elem(*it);
    for(unsigned int n=0; n<elem->n_nodes(); n++)
    {
      Node * node = elem->get_node(n);

      _semilocal_node_list.insert(node);
    }
  }

  delete _active_semilocal_node_range;

  // Now create the actual range
  _active_semilocal_node_range = new SemiLocalNodeRange(_semilocal_node_list.begin(), _semilocal_node_list.end());
}

void
MooseMesh::buildNodeList()
{
  freeBndNodes();

  /// Boundary node list (node ids and corresponding side-set ids, arrays always have the same length)
  std::vector<unsigned int> nodes;
  std::vector<short int> ids;
  _mesh.boundary_info->build_node_list(nodes, ids);

  int n = nodes.size();
  _bnd_nodes.resize(n);
  for (int i = 0; i < n; i++)
    _bnd_nodes[i] = new BndNode(&_mesh.node(nodes[i]), ids[i]);
}

ConstElemRange *
MooseMesh::getActiveLocalElementRange()
{
  if (!_active_local_elem_range)
  {
    _active_local_elem_range = new ConstElemRange(_mesh.active_local_elements_begin(),
                                                  _mesh.active_local_elements_end(), GRAIN_SIZE);
  }

  return _active_local_elem_range;
}

NodeRange *
MooseMesh::getActiveNodeRange()
{
  if (!_active_node_range)
  {
    _active_node_range = new NodeRange(_mesh.active_nodes_begin(),
                                       _mesh.active_nodes_end(), GRAIN_SIZE);
  }

  return _active_node_range;
}

SemiLocalNodeRange *
MooseMesh::getActiveSemiLocalNodeRange()
{
  mooseAssert(_active_semilocal_node_range, "_active_semilocal_node_range has not been created yet!");
/*
  if (!_active_node_range)
  {
    _active_semilocal_node_range = new NodeRange(_mesh.local_nodes_begin(),
                                                 _mesh.local_nodes_end(), GRAIN_SIZE);
  }
 */

  return _active_semilocal_node_range;
}

ConstNodeRange *
MooseMesh::getLocalNodeRange()
{
  if (!_local_node_range)
  {
    _local_node_range = new ConstNodeRange(_mesh.local_nodes_begin(),
                                           _mesh.local_nodes_end(), GRAIN_SIZE);
  }

  return _local_node_range;
}

ConstBndNodeRange *
MooseMesh::getBoundaryNodeRange()
{
  if (!_bnd_node_range)
  {
    _bnd_node_range = new ConstBndNodeRange(bnd_nodes_begin(),
                                            bnd_nodes_end(), GRAIN_SIZE);
  }

  return _bnd_node_range;
}

// MooseMesh Modifiers /////
void
MooseMesh::addMeshModifer(const std::string & mod_name, const std::string & name, InputParameters parameters)
{
  _mesh_modifiers.push_back(static_cast<MeshModifier *>(Factory::instance()->create(mod_name, name, parameters)));
}

void
MooseMesh::applyMeshModifications()
{
  for (std::vector<MeshModifier *>::iterator i = _mesh_modifiers.begin(); i != _mesh_modifiers.end(); ++i)
    (*i)->modifyMesh(_mesh);
}

void
MooseMesh::cacheInfo()
{
  for (MeshBase::element_iterator el = _mesh.local_elements_begin(); el != _mesh.local_elements_end(); ++el)
  {
    Elem * elem = *el;
    for(unsigned int nd = 0; nd < elem->n_nodes(); ++nd)
    {
      Node & node = *elem->get_node(nd);
      _block_node_list[node.id()].insert(elem->subdomain_id());
    }
  }
}

std::set<subdomain_id_type> &
MooseMesh::getNodeBlockIds(const Node & node)
{
  return _block_node_list[node.id()];
}


// default begin() accessor
bnd_node_iterator
MooseMesh::bnd_nodes_begin ()
{
  Predicates::NotNull<bnd_node_iterator_imp> p;
  return bnd_node_iterator(_bnd_nodes.begin(), _bnd_nodes.end(), p);
}

// default end() accessor
bnd_node_iterator
MooseMesh::bnd_nodes_end ()
{
  Predicates::NotNull<bnd_node_iterator_imp> p;
  return bnd_node_iterator(_bnd_nodes.end(), _bnd_nodes.end(), p);
}
