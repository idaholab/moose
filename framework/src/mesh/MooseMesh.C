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
#include "parallel.h"
#include "mesh_communication.h"

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
  // If we are using a truly Parallel mesh (like Nemesis) then we might not even have neighbors!
  if(parallel())
    MeshCommunication().gather_neighboring_elements(_mesh);
  
  _mesh.prepare_for_use(false);

  // If using ParallelMesh this will delete non-local elements from the current processor
  // If using SerialMesh, this function is a no-op.
  _mesh.delete_remote_elements();

  // Collect (local) subdomain IDs
  const MeshBase::element_iterator el_end = _mesh.elements_end();
  for (MeshBase::element_iterator el = _mesh.elements_begin(); el != el_end; ++el)
    _mesh_subdomains.insert((*el)->subdomain_id());

  // Collect (local) boundary IDs
  const std::set<short>& local_bids = _mesh.boundary_info->get_boundary_ids();
  _mesh_boundary_ids.insert(local_bids.begin(), local_bids.end());

  // Communicate subdomain and boundary IDs if this is a parallel mesh
  if (!_mesh.is_serial())
    {
      // Subdomain size before
      // std::cout << "(before) _mesh_subdomains.size()=" << _mesh_subdomains.size() << std::endl;
      
      // Pack our subdomain IDs into a vector
      std::vector<subdomain_id_type> mesh_subdomains_vector(_mesh_subdomains.begin(), 
							    _mesh_subdomains.end());

      // Gather them all into an enlarged vector
      Parallel::allgather(mesh_subdomains_vector);

      // Attempt to insert any new IDs into the set (any existing ones will be skipped)
      _mesh_subdomains.insert(mesh_subdomains_vector.begin(), 
			      mesh_subdomains_vector.end());

      // Subdomain size after
      // std::cout << "(after) _mesh_subdomains.size()=" << _mesh_subdomains.size() << std::endl;




      // Boundary ID size before
      // std::cout << "(before) _mesh_boundary_ids.size()=" << _mesh_boundary_ids.size() << std::endl;

      // Pack our boundary IDs into a vector for communication
      std::vector<short> mesh_boundary_ids_vector(_mesh_boundary_ids.begin(),
						  _mesh_boundary_ids.end());

      // Gather them all into an enlarged vector
      Parallel::allgather(mesh_boundary_ids_vector);

      // Attempt to insert any new IDs into the set (any existing ones will be skipped)
      _mesh_boundary_ids.insert(mesh_boundary_ids_vector.begin(),
				mesh_boundary_ids_vector.end());
      
      // Boundary ID size after
      // std::cout << "(after) _mesh_boundary_ids.size()=" << _mesh_boundary_ids.size() << std::endl;
    }

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

#ifdef LIBMESH_ENABLE_AMR
void
MooseMesh::uniformlyRefine(int level)
{
  MeshRefinement mesh_refinement(_mesh);
  if (level)
    mesh_refinement.uniformly_refine(level);
  else if (_init_refinement_level)
    mesh_refinement.uniformly_refine(_init_refinement_level);
}
#endif //LIBMESH_ENABLE_AMR

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

  // First add the nodes connected to local elems
  ConstElemRange * active_local_elems = getActiveLocalElementRange();
  for(ConstElemRange::const_iterator it=active_local_elems->begin();
      it!=active_local_elems->end();
      ++it)
  {
    const Elem * elem = *it;
    for(unsigned int n=0; n<elem->n_nodes(); n++)
    {
      Node * node = elem->get_node(n);

      _semilocal_node_list.insert(node);
    }
  }
  
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
