//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUELMesh.h"
#include "MooseUtils.h"
#include "MooseMeshUtils.h"

registerMooseObject("SolidMechanicsApp", AbaqusUELMesh);

InputParameters
AbaqusUELMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.addClassDescription(
      "Loads an Abaqus input with custom user elements and manages the resulting mesh. UEL meshes "
      "cannot generally be represented by libMesh meshes. This mesh manages the custom "
      "connectivity and DOF assignment.");
  params.addRequiredParam<FileName>("file", "The path to the Abaqus input to read.");
  params.addParam<bool>("debug", false, "Enable additional debug output.");
  return params;
}

AbaqusUELMesh::AbaqusUELMesh(const InputParameters & parameters)
  : MooseMesh(parameters), _debug(getParam<bool>("debug"))
{
}

AbaqusUELMesh::AbaqusUELMesh(const AbaqusUELMesh & other_mesh)
  : MooseMesh(other_mesh), _debug(other_mesh._debug)
{
}

std::unique_ptr<MooseMesh>
AbaqusUELMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
}

void
AbaqusUELMesh::buildMesh()
{
  // open Abaqus input
  std::ifstream in;
  in.open(getParam<FileName>("file").c_str(), std::ios::in);
  if (!in.good())
    paramError("file", "Error opening mesh file.");
  _input.parse(in);

  // build data structures
  _root.process(_input);

  // instantiate elements
  instantiateElements();

  // create blocks to restrict each variable
  setupLibmeshSubdomains();

  // setup libmesh node sets for abaqus node sets
  setupNodeSets();

  _mesh->prepare_for_use();

  // get set of all subdomain IDs
  for (const auto & elem :
       as_range(_mesh->active_local_elements_begin(), _mesh->active_local_elements_end()))
    _uel_block_ids.insert(elem->subdomain_id());
}

bool
AbaqusUELMesh::prepare(const MeshBase * mesh_to_clone)
{
  // // set the correct processor ID for each element
  // for (const auto i : index_range(_elements))
  //   _elements[i].pid = elemPtr(_elements[i].nodes[i %
  //   _elements[i].nodes.size()])->processor_id();

  // if (_debug)
  // {
  //   for (auto & elem : _elements)
  //     _console << elem.pid << ' ';
  //   _console << std::endl;
  // }

  return MooseMesh::prepare(mesh_to_clone);
}

void
AbaqusUELMesh::instantiateElements()
{
  // add mesh points (libmesh node elements)
  for (const auto id : index_range(_mesh_points))
  {
    const auto & [p, mask] = _mesh_points[id];

    // add the point node and element
    auto * node = _mesh->add_point(p, id);
    auto node_elem = Elem::build(NODEELEM);
    node_elem->set_node(0) = node;
    node_elem->set_id() = id;
    node_elem->subdomain_id = mask;
    _mesh->add_elem(std::move(node_elem));
  }
}

void
AbaqusUELMesh::setupLibmeshSubdomains()
{

  // // iterate over all elements
  // for (const auto & elem_id : index_range(_elements))
  // {
  //   const auto & elem = _elements[elem_id];
  //   const auto & nodes = elem.nodes;
  //   const auto & uel = _element_definition[elem.type_id];

  //   for (const auto i : index_range(nodes))
  //   {
  //     // build node to elem map
  //     _node_to_uel_map[nodes[i]].push_back(elem_id);
  //   }
  // }
}

void
AbaqusUELMesh::setupNodeSets()
{
  BoundaryInfo & boundary_info = _mesh->get_boundary_info();

  // Get the BoundaryIDs from the mesh
  std::vector<BoundaryName> nodeset_names;
  for (const auto & pair : _node_set)
    nodeset_names.push_back(pair.first);
  std::vector<boundary_id_type> nodeset_ids =
      MooseMeshUtils::getBoundaryIDs(*_mesh, nodeset_names, true);

  for (const auto & i : index_range(nodeset_names))
  {
    // add nodes
    const auto & set = _node_set[nodeset_names[i]];
    for (const auto node_id : set)
      boundary_info.add_node(_mesh->query_node_ptr(node_id), nodeset_ids[i]);

    // set names
    boundary_info.nodeset_name(nodeset_ids[i]) = nodeset_names[i];
  }
}

const Abaqus::UserElement &
AbaqusUELMesh::getUEL(const std::string & type) const
{
  for (const auto & part : _root._part)
    if (part._element_definition.has(type))
      return part._element_definition[type];
  mooseError("Unknown UEL type '", type, "'");
}

// const std::vector<std::size_t> &
// AbaqusUELMesh::getNodeSet(const std::string & nset) const
// {
//   const auto it = _node_set.find(MooseUtils::toUpper(nset));
//   if (it == _node_set.end())
//     mooseError("Node set '", nset, "' does not exist.");
//   return it->second;
// }

// const std::vector<std::size_t> &
// AbaqusUELMesh::getElementSet(const std::string & elset) const
// {
//   const auto it = _element_set.find(MooseUtils::toUpper(elset));
//   if (it == _element_set.end())
//     mooseError("Element set '", elset, "' does not exist.");
//   return it->second;
// }

std::string
AbaqusUELMesh::getVarName(std::size_t id) const
{
  static const char coord[] = {'x', 'y', 'z'};
  if (id < 3)
    return std::string("disp_") + coord[id];
  if (id < 6)
    return std::string("rot_") + coord[id - 3];

  return "var_" + Moose::stringify(id + 1);
}

void
AbaqusUELMesh::addNodeset(BoundaryID id)
{
  _mesh_nodeset_ids.insert(id);
}
