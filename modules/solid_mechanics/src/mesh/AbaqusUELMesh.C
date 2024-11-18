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
  return params;
}

AbaqusUELMesh::AbaqusUELMesh(const InputParameters & parameters) : MooseMesh(parameters) {}
AbaqusUELMesh::AbaqusUELMesh(const AbaqusUELMesh & other_mesh) : MooseMesh(other_mesh) {}

std::unique_ptr<MooseMesh>
AbaqusUELMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
}

std::string
AbaqusUELMesh::readLine()
{
  std::string s;
  while (true)
  {
    // read line
    std::getline(*_in, s);

    // check for error condition
    if (*_in)
    {
      // skip empty line
      if (s == "")
        continue;

      // skip comment line
      if (s.substr(0, 2) == "**")
        continue;

      return s;
    }

    // regular end-of-file
    if (_in->eof())
      throw EndOfAbaqusInput();

    // bad stream
    paramError("file", "Error reading file (bad stream).");
  }
}

bool
AbaqusUELMesh::readDataLine(std::string & s)
{
  s.clear();

  while (true)
  {
    if (_in->peek() == '*' || _in->peek() == EOF)
    {
      if (s.empty())
        return false;
      mooseError("Incomplete data line.");
    }

    // read line
    auto l = readLine();
    strip(l);
    s += l;

    // check if line continuation is needed
    if (s.back() != ',')
      return true;
  }
}

bool
AbaqusUELMesh::startsWith(const std::string & s, const std::string & pattern)
{
  const auto n = pattern.length();
  return (s.substr(0, n) == pattern);
}

void
AbaqusUELMesh::strip(std::string & line)
{
  line.erase(std::remove_if(
                 line.begin(), line.end(), [](unsigned char const c) { return std::isspace(c); }),
             line.end());
}

void
AbaqusUELMesh::buildMesh()
{
  // open Abaqus input
  auto inf = std::make_unique<std::ifstream>();
  inf->open(getParam<FileName>("file").c_str(), std::ios::in);
  if (!inf->good())
    paramError("file", "Error opening mesh file.");
  _in = std::move(inf);

  // read file line by line
  std::string s;
  while (true)
  {
    // read line
    try
    {
      s = readLine();
    }
    catch (EndOfAbaqusInput &)
    {
      break;
    }

    // parse the current line
    std::string upper = MooseUtils::toUpper(s);
    try
    {
      if (startsWith(upper, "*NODE"))
      {
        readNodes();
        continue;
      }
      if (startsWith(upper, "*USER ELEMENT"))
      {
        readUserElement(upper);
        continue;
      }
      if (startsWith(upper, "*ELEMENT"))
      {
        readElements(upper);
        continue;
      }
    }
    catch (EndOfAbaqusInput &)
    {
      paramError("file", "Unexpected end of file.");
    }
  }

  // create blocks to restrict each variable
  setupNodeSets();

  _mesh->prepare_for_use();

  // get set of all subdomain IDs
  for (const auto & elem :
       as_range(_mesh->active_local_elements_begin(), _mesh->active_local_elements_end()))
    _uel_block_ids.insert(elem->subdomain_id());
}

void
AbaqusUELMesh::readNodes()
{
  // We will read nodes until the next line begins with *, since that will be the
  // next section.
  while (_in->peek() != '*' && _in->peek() != EOF)
  {
    // read and split line
    std::string s = readLine();
    strip(s);
    std::vector<std::string> col;
    MooseUtils::tokenize(s, col, 1, ",");

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
                    map.get<int>("variables"),
                    map.get<int>("properties"),
                    !map.get<bool>("unsym"),
                    type,
                    std::vector<std::vector<std::size_t>>(n_nodes)};

  // We will read nodes until the next line begins with *
  std::string s;
  while (readDataLine(s))
  {
    // split line
    std::vector<std::size_t> col;
    MooseUtils::tokenizeAndConvert(s, col, ",");
    const auto node_number = col[0] - 1;
    if (node_number >= n_nodes)
      paramError("file", "Invalid node number in Abaqus input.");

    // copy in var numbers (converting from 1-base to 0-base)
    auto & var = uel.vars[node_number];
    for (const auto i : index_range(col))
      if (i > 0)
        var.push_back(col[i] - 1);
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

  // We will read nodes until the next line begins with *
  std::string s;
  while (readDataLine(s))
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
    UserElement elem{type_id, elem_id, {}};

    // make room in the vector
    if (elem_id >= _elements.size())
      _elements.resize(elem_id + 1);

    // copy in node numbers (converting from 1-base to 0-base)
    for (const auto i : index_range(col))
      if (i > 0)
        elem.nodes.push_back(col[i] - 1);

    _elements[elem_id] = elem;
  }
}

void
AbaqusUELMesh::setupNodeSets()
{
  // verify variable numbers are below number of bits in BoundaryID
  const auto bits = sizeof(SubdomainID) * 8;
  for (const auto & uel : _element_definition)
    for (const auto & nodes : uel.vars)
      for (const auto & var : nodes)
        if (var >= bits)
          mooseError("Currently variables numbers >= ", bits, " are not supported.");

  // iterate over all elements
  for (const auto & [type_id, elem_id, nodes] : _elements)
  {
    const auto & uel = _element_definition[type_id];

    for (const auto i : index_range(nodes))
    {
      // build node to elem map
      _node_to_uel_map[nodes[i]].push_back(elem_id);

      // add node element to variable-specific block
      auto * elem = _mesh->elem_ptr(nodes[i]);
      for (const auto & var : uel.vars[nodes[i]])
      {
        auto id = elem->subdomain_id();
        id = id | (1 << var);
        elem->subdomain_id() = id;
      }
      // for (const auto & var : uel.vars[nodes[i]])
      //   elem->subdomain_id() |= (1 << var);
    }
  }
}

const AbaqusUELMesh::UELDefinition &
AbaqusUELMesh::getUEL(const std::string & type)
{
  const auto it = _element_type_to_typeid.find(type);
  if (it == _element_type_to_typeid.end())
    mooseError("Unknown UEL type '", type, "'");
  return _element_definition[it->second];
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
HeaderMap::has(const std::string & key)
{
  return _map.find(MooseUtils::toUpper(key)) != _map.end();
}

template <>
bool
HeaderMap::get(const std::string & key)
{
  const auto it = _map.find(MooseUtils::toUpper(key));
  return it != _map.end();
}
