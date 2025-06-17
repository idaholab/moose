//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusInputObjects.h"
#include "AbaqusUELMesh.h"
#include "MooseError.h"
#include "MooseUtils.h"
#include "MooseMeshUtils.h"
#include "libMeshReducedNamespace.h"
#include <memory>
#include <string>

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

  if (_input.isFlat())
  {
    mooseInfo("Flat input file format detected.");
    _model = std::make_unique<Abaqus::FlatModel>();
  }
  else
  {
    mooseInfo("Hierarchical input file format detected.");
    _model = std::make_unique<Abaqus::AssemblyModel>();
  }

  // build data structures
  _model->parse(_input);

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
  _mesh->reserve_nodes(_model->_nodes.size());
  for (const auto node_index : index_range(_model->_nodes))
  {
    const auto & [abaqus_id, p, mask] = _model->_nodes[node_index];
    libmesh_ignore(abaqus_id); // (use extra element id for this?)

    // add the point node and element
    auto * node = _mesh->add_point(p, node_index);
    auto node_elem = Elem::build(NODEELEM);
    node_elem->set_node(0) = node;
    node_elem->set_id() = node_index;
    node_elem->subdomain_id() = mask;
    _uel_block_ids.insert(mask);
    _mesh->add_elem(std::move(node_elem));
  }
}

void
AbaqusUELMesh::setupLibmeshSubdomains()
{

  // iterate over all elements
  for (const auto & elem_index : index_range(_model->_elements))
  {
    const auto & elem = _model->_elements[elem_index];
    const auto & nodes = elem._nodes;
    // const auto & uel = elem._uel;

    // build node to elem map
    for (const auto i : index_range(nodes))
      _node_to_uel_map[nodes[i]].push_back(elem_index);
  }
}

void
AbaqusUELMesh::setupNodeSets()
{
  BoundaryInfo & boundary_info = _mesh->get_boundary_info();

  // Get the BoundaryIDs from the mesh
  std::vector<BoundaryName> nodeset_names = {"abaqus_bc_union_boundary"};
  for (const auto & pair : _node_set)
    nodeset_names.push_back(pair.first);

  std::vector<boundary_id_type> nodeset_ids =
      MooseMeshUtils::getBoundaryIDs(*_mesh, nodeset_names, true);

  // add BC nodes to unified set
  auto add_bc_nodes = [&](const auto & step)
  {
    for (const auto & node_value_map : step._bc_var_node_value_map)
      for (const auto node_value : node_value_map.second)
      {
        const auto node = _mesh->query_node_ptr(node_value.first);
        if (!node)
          mooseError("Node ", node_value.first, " not found\n");
        boundary_info.add_node(node, nodeset_ids[0]);
      }
  };

  for (const auto i : index_range(nodeset_names))
    boundary_info.nodeset_name(nodeset_ids[i]) = nodeset_names[i];

  add_bc_nodes(*_model);
  for (const auto & step : _model->_step)
    add_bc_nodes(step);

  _mesh->set_isnt_prepared();
}

const Abaqus::UserElement &
AbaqusUELMesh::getUEL(const std::string & type) const
{
  if (_model->_element_definition.has(type))
    return _model->_element_definition[type];
  mooseError("Unknown UEL type '", type, "'");
}

std::string
AbaqusUELMesh::getVarName(const Abaqus::AbaqusID var_id) const
{
  if (var_id < 1)
    mooseError("Abaqus variables IDs start at 1");

  static const char coord[] = {'x', 'y', 'z'};
  if (var_id <= 3)
    return std::string("disp_") + coord[var_id - 1];
  if (var_id <= 6)
    return std::string("rot_") + coord[var_id - 4];

  return "var_" + Moose::stringify(var_id);
}

void
AbaqusUELMesh::addNodeset(BoundaryID id)
{
  _mesh_nodeset_ids.insert(id);
}

const std::unordered_map<Abaqus::Index, Real> &
AbaqusUELMesh::getBCFor(const Abaqus::AbaqusID var_id)
{
  // accidental insertion... or should we throw an error
  return _model->_bc_var_node_value_map[var_id];
}

const std::unordered_map<Abaqus::Index, Real> &
AbaqusUELMesh::getBCFor(const Abaqus::AbaqusID var_id, const std::string & step_name)
{
  // accidental insertion... or should we throw an error
  return _model->_step[step_name]._bc_var_node_value_map[var_id];
}
