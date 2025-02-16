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

  //
  // parts
  //
  _input.forBlock("part",
                  [](auto block)
                  {
                    std::cout << block._type << '\n';
                    return;
                  });

  //
  // assembly
  _input.forBlock("assembly",
                  [](auto block)
                  {
                    block.forBlock("instance",
                                   [](auto block)
                                   {
                                     std::cout << block._type << '\n';
                                     return;
                                   });

                    block.forOption("nset",
                                    [](auto option)
                                    {
                                      std::cout << option._type << '\n';
                                      return;
                                    });
                  });

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
AbaqusUELMesh::readNodes()
{
  for (const auto & col : std::vector<std::vector<std::string>>())
  {
    // // read nodes
    // if (col == "node")
    //   readNodes();
    // else if (col == "nset")
    //   readNodeSet();

    // node id
    int id = MooseUtils::convert<int>(col[0]) - 1;

    // check that we don't have too many coordinate components
    if (col.size() > 4)
      paramError("file", "Node coordinates with more than 3 components encountered in input.");

    // parse coordinates
    Point p;
    for (const auto i : make_range(col.size() - 1))
      p(i) = MooseUtils::convert<Real>(col[i + 1]);

    // add the point with the original Abaqus id
    auto * node = _mesh->add_point(p, id);
    auto node_elem = Elem::build(NODEELEM);
    node_elem->set_node(0) = node;
    node_elem->set_id() = id;
    _mesh->add_elem(std::move(node_elem));

    // keep track of largest node ID
    if (id > _max_node_id)
      _max_node_id = id;
  }
}

void
AbaqusUELMesh::readUserElement(const std::string & header)
{
  // parse the header line
  HeaderMap map(header);
  const auto n_nodes = map.get<std::size_t>("nodes");
  const auto type = map.get<std::string>("type");

  UELDefinition uel{map.get<int>("coordinates"),
                    n_nodes,
                    map.has("variables") ? map.get<int>("variables") : 1,
                    map.has("properties") ? map.get<int>("properties") : 0,
                    map.has("i properties") ? map.get<int>("i properties") : 0,
                    !map.get<bool>("unsym"),
                    type,
                    _element_definition.size(),
                    std::vector<std::vector<std::size_t>>(n_nodes)};

  // We will read nodes until the next line begins with *
  std::string s;
  int line = 0;
  std::set<std::size_t> seen_node;
  while (false)
  {
    // split line
    std::vector<std::size_t> col;
    MooseUtils::tokenizeAndConvert(s, col, ",");

    // number of the current node (0-based)
    const auto node_number = line ? (col[0] - 1) : 0;
    if (node_number >= n_nodes)
      paramError("file", "Invalid node number in Abaqus input.");

    // mark off node as seen and check for duplicates
    if (seen_node.count(node_number))
      mooseError("Duplicate node in '*user element' section.");
    seen_node.insert(node_number);

    // copy in var numbers (converting from 1-base to 0-base)
    auto & var = uel.vars[node_number];
    for (const auto i : index_range(col))
      // note how abaqus treats the first data line differnetly from the following lines!
      if (i > 0 || !line)
        var.push_back(col[i] - 1);

    line++;
  }

  // check if any node numbers were omitted, and if so, set them to the preceeding node's variables
  for (const auto i : make_range(n_nodes - 1))
    if (!seen_node.count(i + 1))
      for (const auto v : uel.vars[i])
        uel.vars[i + 1].push_back(v);

  // debug output
  if (_debug)
  {
    std::stringstream ss;
    ss << "UEL '" << type << "':\n";
    for (const auto i : make_range(n_nodes))
      ss << i << ": " << Moose::stringify(uel.vars[i]) << '\n';
    mooseInfoRepeated(ss.str());
  }

  // insert custom element into map
  if (_element_type_to_typeid.find(type) != _element_type_to_typeid.end())
    paramError("file", "Duplicate user element definition in Abaqus input.");
  _element_type_to_typeid[type] = _element_definition.size();
  _element_definition.push_back(uel);
}

void
AbaqusUELMesh::readElements(const std::string & header)
{
  // parse the header line
  HeaderMap map(header);
  const auto type = map.get<std::string>("type");
  const auto it = _element_type_to_typeid.find(type);
  if (it == _element_type_to_typeid.end())
    paramError("file", "Unknown user element type '", type, "' in Abaqus input.");
  const auto type_id = it->second;

  // add new elements to an element set?
  auto * elset = map.has("elset") ? &_element_set[map.get<std::string>("elset")] : nullptr;
  std::set<std::size_t> unique_ids;
  if (elset)
    unique_ids.insert(elset->begin(), elset->end());

  // We will read nodes until the next line begins with *
  std::string s;
  while (false)
  {
    // split line
    std::vector<std::size_t> col;
    MooseUtils::tokenizeAndConvert(s, col, ",");
    const auto elem_id = col[0] - 1;

    // check number of nodes
    if (col.size() - 1 != _element_definition[type_id].nodes)
      paramError(
          "file", "Wrong number of nodes for user element of type '", type, "' in Abaqus input.");

    // prepare empty element
    UserElement elem{type_id, DofObject::invalid_processor_id, {}, {nullptr, nullptr}};

    // make room in the vector
    if (elem_id >= _elements.size())
      _elements.resize(elem_id + 1);

    // copy in node numbers (converting from 1-base to 0-base)
    for (const auto i : index_range(col))
      if (i > 0)
        elem.nodes.push_back(col[i] - 1);

    _elements[elem_id] = elem;

    if (elset)
      unique_ids.insert(elem_id);
  }

  if (elset)
    elset->assign(unique_ids.begin(), unique_ids.end());
}

void
AbaqusUELMesh::readNodeSet(const std::string & header)
{
  HeaderMap map(header);
  if (map.has("elset"))
  {
    // copy nodes from element set
    const auto & elset = getElementSet(map.get<std::string>("elset"));
    auto & nset = _node_set[map.get<std::string>("nset")];
    std::set<std::size_t> unique_nodes(nset.begin(), nset.end());
    for (const auto elem_id : elset)
      for (const auto node_id : _elements[elem_id].nodes)
        unique_nodes.insert(node_id);
    nset.assign(unique_nodes.begin(), unique_nodes.end());
  }
  else
    readSetHelper(_node_set, map, "nset");
}

void
AbaqusUELMesh::readElementSet(const std::string & header)
{
  HeaderMap map(header);
  readSetHelper(_element_set, map, "elset");
}

void
AbaqusUELMesh::readSetHelper(std::map<std::string, std::vector<std::size_t>> & set_map,
                             const HeaderMap & map,
                             const std::string & name_key)
{
  // parse the header line
  const auto name = map.get<std::string>(name_key);
  if (set_map.count(name))
    paramError("file", "Repeated ", name_key, " declaration for '", name, "' in Abaqus input.");

  // implement GENERATE keyword
  const auto generate = map.get<bool>("generate");

  if (map.has("unsorted"))
    paramError("file", "The UNSORTED keyword is not supported.");

  // We will read data lines until the next line begins with *
  std::string s;
  auto & set = set_map[name];
  std::set<std::size_t> unique_items(set.begin(), set.end());
  while (false)
  {
    // split line
    std::vector<std::string> col;
    MooseUtils::tokenize(s, col, 1, ",");

    if (generate)
    {
      // syntax check
      if (col.size() != 3)
        paramError("file",
                   "Expected three values in ",
                   name_key,
                   " definition '",
                   name,
                   "' with GENERATE keyword in Abaqus input.");

      // generate range
      const auto begin = MooseUtils::convert<std::size_t>(col[0]);
      const auto end = MooseUtils::convert<std::size_t>(col[1]);
      const auto step = MooseUtils::convert<std::size_t>(col[2]);
      if (step == 0)
        paramError("file", "Zero step in generated set.");
      for (std::size_t item = begin; item <= end; item += step)
        unique_items.insert(item - 1);
    }
    else
    {
      for (const auto i : index_range(col))
      {
        // check for existing set first
        const auto it = set_map.find(col[i]);
        if (it != set_map.end())
        {
          // insert existing set
          unique_items.insert(it->second.begin(), it->second.end());
        }
        else
        {
          const auto item = MooseUtils::convert<std::size_t>(col[i]);
          if (item > 0)
            unique_items.insert(item - 1);
          else
            paramError("file", "Invalid ID in ", name_key, " '", name, "' in Abaqus input.");
        }
      }
    }
  }

  set.assign(unique_items.begin(), unique_items.end());
  if (_debug)
    mooseInfoRepeated(name_key, " '", name, "': ", Moose::stringify(set));
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

AbaqusUELMesh::AbaqusInputBlock
AbaqusUELMesh::readInputBlock(const std::string & header)
{
  AbaqusInputBlock block(header);
  std::string s;
  while (false)
    block._data_lines.push_back(s);
  if (_debug)
    mooseInfoRepeated("Block: ", header, "\n", Moose::stringify(block._data_lines));
  return block;
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

HeaderMap::HeaderMap(const std::string & header) : _header(header)
{
  std::vector<std::string> fields;
  MooseUtils::tokenize(header, fields, 1, ",");
  fields.erase(fields.begin());
  for (const auto & field : fields)
  {
    const auto begin = field.find_first_of("=");
    if (begin == std::string::npos)
      _map[field] = "";
    else
    {
      const auto key = MooseUtils::toUpper(MooseUtils::trim(field.substr(0, begin)));
      const auto value = MooseUtils::trim(field.substr(begin + 1));
      _map[key] = value;
    }
  }
}

bool
HeaderMap::has(const std::string & key) const
{
  return _map.find(MooseUtils::toUpper(key)) != _map.end();
}

template <>
bool
HeaderMap::get(const std::string & key) const
{
  const auto it = _map.find(MooseUtils::toUpper(key));
  return it != _map.end();
}
