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
  // set the correct processor ID for each element
  for (const auto i : index_range(_elements))
    _elements[i].pid = elemPtr(_elements[i].nodes[i % _elements[i].nodes.size()])->processor_id();

  if (_debug)
  {
    for (auto & elem : _elements)
      _console << elem.pid << ' ';
    _console << std::endl;
  }

  return MooseMesh::prepare(mesh_to_clone);
}

void
AbaqusUELMesh::instantiateElements()
{
  // let's hope 0 is ok here...
  _max_node_id = 0;

  for (const auto & assembly : _root._assembly)
    for (const auto & instance : assembly._instance)
    {
      // part to instantiate
      const auto & part = _root._part[instance._part_id];

      // loop over nodes
      for (const auto & [abaqus_node_id, pp] : part._node)
      {
        // transform part point pp to instance point ip
        Point ip = instance._rotation * (pp + instance._transform);

        // add the point node and element
        auto * node = _mesh->add_point(ip, _max_node_id);
        auto node_elem = Elem::build(NODEELEM);
        node_elem->set_node(0) = node;
        node_elem->set_id() = _max_node_id;
        _mesh->add_elem(std::move(node_elem));
        _part._libmesh_node_id[abaqus_node_id] = _max_node_id;
        _max_node_id++;
      }

      // add elements
      for (const auto & [abaqus_elem_id, abaqus_elem] : part._element)
      {
        LibMeshUElement elem(part._element_definition[abaqus_elem._type_id]);
        // translate abaqus node ids to libmesh node ids
        for (const auto abaqus_node_id : abaqus_elem._node_list )
          elem._libmesh_node_list.push_back(_part._libmesh_node_id[abaqus_node_id]);

         _part._moose_elem_id[abaqus_elem_id] = _elements.size();
         _elements.push_back(elem);
      }

      // deal with elset/nset at the part and root level
    }
}

void
AbaqusUELMesh::readProperties(const std::string & header)
{
  // process header
  HeaderMap map(header);
  const auto & elset = getElementSet(map.get<std::string>("elset"));

  // read data lines
  _properties.emplace_back();
  auto & props = _properties.back();

  std::string s;
  while (false)
  {
    // tokenize all data as both integer and float. this should always succeed. we leter iterate
    // over elements and only then know from the uel type how many entries are float and int.
    std::vector<Real> rcol;
    std::vector<int> icol;
    MooseUtils::tokenizeAndConvert(s, rcol, ",");
    props.first.insert(props.first.end(), rcol.begin(), rcol.end());
    MooseUtils::tokenizeAndConvert(s, icol, ",");
    props.second.insert(props.second.end(), icol.begin(), icol.end());
  }

  // assign properties to elements
  for (const auto uel_id : elset)
  {
    auto & elem = _elements[uel_id];
    const auto & uel = _element_definition[elem.type_id];
    if (uel.n_properties > 0)
      elem.properties.first = props.first.data();
    if (uel.n_iproperties > 0)
      elem.properties.second = &(props.second[uel.n_properties]);
  }
}

void
AbaqusUELMesh::setupLibmeshSubdomains()
{
  // verify variable numbers are below number of bits in BoundaryID
  const auto bits = sizeof(SubdomainID) * 8;
  for (const auto & uel : _element_definition)
    for (const auto & nodes : uel.vars)
      for (const auto & var : nodes)
        if (var >= bits)
          mooseError("Currently variables numbers >= ", bits, " are not supported.");

  // iterate over all elements
  for (const auto & elem_id : index_range(_elements))
  {
    const auto & elem = _elements[elem_id];
    const auto & nodes = elem.nodes;
    const auto & uel = _element_definition[elem.type_id];

    for (const auto i : index_range(nodes))
    {
      // build node to elem map
      _node_to_uel_map[nodes[i]].push_back(elem_id);

      // add node element to variable-specific block
      auto * node_elem = _mesh->elem_ptr(nodes[i]);
      if (!node_elem)
        mooseError("Element not found. Internal error.");
      for (const auto & var : uel.vars[i])
        node_elem->subdomain_id() |= (1 << var);
    }
  }
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

const AbaqusUELMesh::UELDefinition &
AbaqusUELMesh::getUEL(const std::string & type) const
{
  const auto it = _element_type_to_typeid.find(type);
  if (it == _element_type_to_typeid.end())
    mooseError("Unknown UEL type '", type, "'");
  return _element_definition[it->second];
}

const std::vector<std::size_t> &
AbaqusUELMesh::getNodeSet(const std::string & nset) const
{
  const auto it = _node_set.find(MooseUtils::toUpper(nset));
  if (it == _node_set.end())
    mooseError("Node set '", nset, "' does not exist.");
  return it->second;
}

const std::vector<std::size_t> &
AbaqusUELMesh::getElementSet(const std::string & elset) const
{
  const auto it = _element_set.find(MooseUtils::toUpper(elset));
  if (it == _element_set.end())
    mooseError("Element set '", elset, "' does not exist.");
  return it->second;
}

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
