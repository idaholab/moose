#include "MooseMesh.h"
#include "Factory.h"
#include "MeshModifier.h"

// libMesh
#include "boundary_info.h"
#include "mesh_tools.h"
#include "mesh_refinement.h"

MooseMesh::MooseMesh(int mesh_dim) :
    _mesh(mesh_dim),
    _is_changed(false),
    _active_local_elem_range(NULL),
    _active_node_range(NULL),
    _local_node_range(NULL),
    _bnd_node_range(NULL)
{
}

MooseMesh::MooseMesh(const MooseMesh & other_mesh) :
    _mesh(other_mesh._mesh),
    _is_changed(false),
    _active_local_elem_range(NULL),
    _active_node_range(NULL),
    _local_node_range(NULL),
    _bnd_node_range(NULL)
{
  (*_mesh.boundary_info) = (*other_mesh._mesh.boundary_info);
}

MooseMesh::~MooseMesh()
{
  delete _active_local_elem_range;
  delete _active_node_range;
  delete _local_node_range;
  delete _bnd_node_range;
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
}

void
MooseMesh::uniformlyRefine(int level)
{
  MeshRefinement mesh_refinement(_mesh);
  mesh_refinement.uniformly_refine(level);
}

void
MooseMesh::meshChanged()
{
  // Rebuild the boundary conditions
  build_node_list_from_side_list();

  // Rebuild the active local element range
  delete _active_local_elem_range;
  _active_local_elem_range = NULL;

  // Rebuild the node range
  delete _active_node_range;
  _active_node_range = NULL;

  // Calling this function will rebuild the range.
  getActiveLocalElementRange();

  // Calling this function will rebuild the range.
  getActiveNodeRange();

  // Print out information about the adapted mesh if requested
//  if (_print_mesh_changed)
//  {
//    std::cout << "\nMesh Changed:\n";
//    _mesh->print_info();
//  }

  //Update the node to elem map
  MeshTools::build_nodes_to_elem_map(_mesh, _node_to_elem_map);

  // Lets the output system know that the mesh has changed recently.
  _is_changed = true;
}

void
MooseMesh::buildBoudndaryNodeList()
{
  _mesh.boundary_info->build_node_list(_bnd_nodes, _bnd_ids);
}

ConstElemRange *
MooseMesh::getActiveLocalElementRange()
{
  if (!_active_local_elem_range)
  {
    _active_local_elem_range = new ConstElemRange(_mesh.active_local_elements_begin(),
                                                  _mesh.active_local_elements_end(), 1);
  }

  return _active_local_elem_range;
}

NodeRange *
MooseMesh::getActiveNodeRange()
{
  if (!_active_node_range)
  {
    _active_node_range = new NodeRange(_mesh.active_nodes_begin(),
                                       _mesh.active_nodes_end(), 1);
  }

  return _active_node_range;
}

ConstNodeRange *
MooseMesh::getLocalNodeRange()
{
  if (!_local_node_range)
  {
    _local_node_range = new ConstNodeRange(_mesh.local_nodes_begin(),
                                           _mesh.local_nodes_end(), 1);
  }

  return _local_node_range;
}

ConstNodeRange *
MooseMesh::getBoundaryNodeRange()
{
  if (!_bnd_node_range)
  {
//    _bnd_node_range = new ConstNodeRange(_bnd_nodes.begin(),
//                                         _bnd_nodes.end(), 1);
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

