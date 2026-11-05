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
#include "AbaqusUELPartitioner.h"
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
  // the UEL connectivity refers to NodeElems by id, so libMesh must not renumber them
  params.set<bool>("allow_renumbering") = false;
  params.suppressParameter<bool>("allow_renumbering");
  return params;
}

AbaqusUELMesh::AbaqusUELMesh(const InputParameters & parameters) : MooseMesh(parameters)
{
  // The libMesh mesh consists of disconnected NodeElems, so the default Metis/ParMETIS
  // partitioners see an empty connectivity graph. Install a partitioner that builds the graph
  // from the UEL connectivity instead. A user-supplied [Partitioner] block still overrides this.
  // The base-class method clones the argument and stores the clone, so this stack object can
  // safely go out of scope (same idiom as PartitionerAction).
  AbaqusUELPartitioner partitioner(*this);
  setCustomPartitioner(&partitioner);

  // the UEL model data is parsed in buildMesh(), so the mesh is rebuilt rather than read from the
  // checkpoint when recovering
  allowRecovery(false);
}

AbaqusUELMesh::AbaqusUELMesh(const AbaqusUELMesh & other_mesh)
  : MooseMesh(other_mesh),
    _model(other_mesh._model),
    _node_to_uel_map(other_mesh._node_to_uel_map),
    _uel_block_ids(other_mesh._uel_block_ids)
{
  AbaqusUELPartitioner partitioner(*this);
  setCustomPartitioner(&partitioner);
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
  Abaqus::InputParser input;
  input.parse(in);

  if (input.isFlat())
  {
    mooseInfo("Flat input file format detected.");
    _model = std::make_shared<Abaqus::FlatModel>();
  }
  else
  {
    mooseInfo("Hierarchical input file format detected.");
    _model = std::make_shared<Abaqus::AssemblyModel>();
  }

  // build data structures
  _model->parse(input);

  // instantiate elements
  instantiateElements();

  // build the map from nodes to the UEL elements that contain them
  buildNodeToUELMap();

  // setup libmesh node sets for abaqus node sets
  setupNodeSets();

  _mesh->prepare_for_use();
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
    node_elem->set_node(0, node);
    node_elem->set_id() = node_index;
    node_elem->subdomain_id() = mask;
    _uel_block_ids.insert(mask);
    _mesh->add_elem(std::move(node_elem));
  }
}

void
AbaqusUELMesh::buildNodeToUELMap()
{
  for (const auto elem_index : index_range(_model->_elements))
    for (const auto node_index : _model->_elements[elem_index]._nodes)
      _node_to_uel_map[node_index].push_back(elem_index);
}

void
AbaqusUELMesh::setupNodeSets()
{
  BoundaryInfo & boundary_info = _mesh->get_boundary_info();

  // Get the BoundaryIDs from the mesh
  const std::vector<BoundaryName> nodeset_names = {"abaqus_bc_union_boundary"};
  const std::vector<boundary_id_type> nodeset_ids =
      MooseMeshUtils::getBoundaryIDs(*_mesh, nodeset_names, true);

  // add BC nodes to unified set
  auto add_bc_nodes = [this, &boundary_info, &nodeset_ids](const auto & step)
  {
    for (const auto & [var_id, node_value_map] : step._bc_var_node_value_map)
    {
      libmesh_ignore(var_id);
      for (const auto & [node_index, value] : node_value_map)
      {
        libmesh_ignore(value);
        const auto node = _mesh->query_node_ptr(node_index);
        if (!node)
          mooseError("Node ", node_index, " not found");
        boundary_info.add_node(node, nodeset_ids[0]);
      }
    }
  };

  for (const auto i : index_range(nodeset_names))
    boundary_info.nodeset_name(nodeset_ids[i]) = nodeset_names[i];

  add_bc_nodes(*_model);
  for (const auto & step : _model->_step)
    add_bc_nodes(step);

  _mesh->unset_is_prepared();
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
