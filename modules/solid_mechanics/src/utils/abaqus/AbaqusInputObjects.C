//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusInputObjects.h"
#include "MooseUtils.h"

#include <stdexcept>

namespace Abaqus
{
template <typename T>
std::vector<T>
vecTo(const std::vector<std::string> & in)
{
  std::vector<T> out;
  std::transform(in.begin(),
                 in.end(),
                 std::back_inserter(out),
                 [](const std::string & str) { return MooseUtils::convert<T>(str); });
  return out;
}

Nodes::Nodes(const OptionNode & option)
{
  for (const auto & data : option._data)
  {
    // node id
    int id = MooseUtils::convert<int>(data[0]);

    // check that we don't have too many coordinate components
    if (data.size() > 4)
      throw std::runtime_error(
          "Node coordinates with more than 3 components encountered in input.");

    // parse coordinates
    Point p;
    for (const auto i : make_range(data.size() - 1))
      p(i) = MooseUtils::convert<Real>(data[i + 1]);

    _node.emplace_back(id, p);
  }
}

UserElement::UserElement(const OptionNode & option)
{
  const auto & map = option._header;
  _coords = map.get<int>("coordinates");
  _n_nodes = map.get<std::size_t>("nodes");
  _n_statev = map.get<int>("variables", 1);
  _n_properties = map.get<int>("properties", 0);
  _n_iproperties = map.get<int>("i properties", 0);
  _symmetric = !map.get<bool>("unsym");
  _type = map.get<std::string>("type");

  // to be filled in later
  _type_id = -1;

  // available bits for the dof mask
  const auto bits = sizeof(SubdomainID) * 8;

  // parse variable (DOF) info
  _vars.resize(_n_nodes);
  _var_mask.resize(_n_nodes);
  std::set<std::size_t> seen_node;
  for (const auto line : index_range(option._data))
  {
    const auto col = vecTo<std::size_t>(option._data[line]);

    // number of the current node (0-based)
    const auto node_number = line ? (col[0] - 1) : 0u;
    if (node_number >= _n_nodes)
      throw std::runtime_error("Invalid node number in Abaqus input.");

    // mark off node as seen and check for duplicates
    if (seen_node.count(node_number))
      throw std::runtime_error("Duplicate node in '*user element' section.");
    seen_node.insert(node_number);

    // copy in var numbers (converting from 1-base to 0-base)
    auto & var = _vars[node_number];
    auto & var_mask = _var_mask[node_number];
    for (const auto i : index_range(col))
      // note how abaqus treats the first data line differently from the following lines!
      if (i > 0 || !line)
      {
        const auto v = col[i] - 1;
        if (v >= bits)
          throw std::runtime_error("Currently variables numbers >= " + Moose::stringify(bits) +
                                   " are not supported.");
        var.push_back(v);
        var_mask |= (1 << v);
      }
  }

  // check if any node numbers were omitted, and if so, set them to the preceeding node's variables
  for (const auto i : make_range(_n_nodes - 1))
    if (!seen_node.count(i + 1))
    {
      _vars[i + 1] = _vars[i];
      _var_mask[i + 1] |= _var_mask[i];
    }
}

Part::Part(const BlockNode & block) : SetContainer(this)
{
  auto option_func =
      [&](const std::string & key, const OptionNode & option)
  {
    // User element definitions
    if (key == "user element")
    {
      const auto type = block._header.get<std::string>("type");
      const auto id = _element_definition.add(type, block);
      _element_definition[id]._type_id = id;
    }

    // Nodes
    if (key == "node")
    {
      const auto & map = option._header;
      if (map.get<std::string>("system", "R") != "R")
        throw std::runtime_error("Only cartesian coordinates are currently supported");

      // this should not be hard to add though
      if (map.has("input"))
        throw std::runtime_error("External coordinate inputs are not yet supported");

      auto * nset = map.has("nset") ? &_node_set[map.get<std::string>("nset")] : nullptr;
      std::set<std::size_t> unique_ids;
      if (elset)
        unique_ids.insert(nset->begin(), nset->end());

      // loop over data lines
      for (const auto & data : option._data) // or loop over external input lines
      {
        if (data.size() < 4)
          throw std::runtime_error("Insufficient data in node data line");
        if (data.size() > 4)
          throw std::runtime_error("Normal directions are not yet supported");

        const auto id = MooseUtils::convert<int>(data[0]);
        Point p(MooseUtils::convert<Real>(data[1]),
                MooseUtils::convert<Real>(data[2]),
                MooseUtils::convert<Real>(data[3]));
        _node.emplace_back(id, p);

        if (nset)
          unique_ids.insert(id);
      }

      if (nset)
        nset->assign(unique_ids.begin(), unique_ids.end());
    }

    // Elements
    if (key == "element")
    {
      const auto & map = option._header;
      const auto type = map.get<std::string>("type");
      const auto it = _element_type_to_typeid.find(type);
      if (it == _element_type_to_typeid.end())
        throw std::runtime_error("Unknown user element type '" + type + "' in Abaqus input.");
      const auto type_id = it->second;

      // add new elements to an element set?
      auto * elset = map.has("elset") ? &_element_set[map.get<std::string>("elset")] : nullptr;
      std::set<std::size_t> unique_ids;
      if (elset)
        unique_ids.insert(elset->begin(), elset->end());

      // loop over data lines
      for (const auto & data : option._data)
      {
        const auto col = vecTo<std::size_t>(data);

        // check number of nodes
        if (col.size() - 1 != _element_definition[type_id]._n_nodes)
          throw std::runtime_error("Wrong number of nodes for user element of type '" + type +
                                   "' in Abaqus input.");

        // prepare empty element
        const auto elem_id = col[0];
        Element elem{type_id, {}};

        // copy in node numbers (converting from 1-base to 0-base)
        for (const auto i : index_range(col))
          if (i > 0)
            elem._node_list.push_back(col[i] - 1);

        // insert into unordered map
        _element[elem_id] = elem;

        if (elset)
          unique_ids.insert(elem_id);
      }

      if (elset)
        elset->assign(unique_ids.begin(), unique_ids.end());
    }

    if (key == "elset")
      processElementSet(option);
    if (key == "nset")
      processNodeSet(option);

    // UEL properties
    if (key == "uel property")
    {
      const auto & map = option._header;
      const auto type = map.get<std::string>("type");

      // // process header
      // HeaderMap map(header);
      // const auto & elset = getElementSet(map.get<std::string>("elset"));

      // // read data lines
      // _properties.emplace_back();
      // auto & props = _properties.back();

      // std::string s;
      // while (false)
      // {
      //   // tokenize all data as both integer and float. this should always succeed. we leter iterate
      //   // over elements and only then know from the uel type how many entries are float and int.
      //   std::vector<Real> rcol;
      //   std::vector<int> icol;
      //   MooseUtils::tokenizeAndConvert(s, rcol, ",");
      //   props.first.insert(props.first.end(), rcol.begin(), rcol.end());
      //   MooseUtils::tokenizeAndConvert(s, icol, ",");
      //   props.second.insert(props.second.end(), icol.begin(), icol.end());
      // }

      // // assign properties to elements
      // for (const auto uel_id : elset)
      // {
      //   auto & elem = _elements[uel_id];
      //   const auto & uel = _element_definition[elem.type_id];
      //   if (uel.n_properties > 0)
      //     elem.properties.first = props.first.data();
      //   if (uel.n_iproperties > 0)
      //     elem.properties.second = &(props.second[uel.n_properties]);
      // }
    }
  }

      block.forAll(option_func, nullptr);
}

void
SetContainer::processNodeSet(const OptionNode & option)
{
  const auto & map = option._header;
  if (map.has("elset"))
  {
    if (!_part)
      throw std::runtime_error("elset option can only be use at the part level");
    // copy nodes from element set
    const auto & elset = getElementSet(map.get<std::string>("elset"));
    auto & nset = _node_set[map.get<std::string>("nset")];
    std::set<std::size_t> unique_nodes(nset.begin(), nset.end());
    for (const auto elem_id : elset)
      for (const auto node_id : _part->_element[elem_id]._node)
        unique_nodes.insert(node_id);
    nset.assign(unique_nodes.begin(), unique_nodes.end());
  }
  else
    processSetHelper(_node_set, option, "nset");
}

void
SetContainer::processElementSet(const OptionNode & option)
{
  processSetHelper(_element_set, option, "elset");
}

const std::vector<std::size_t> &
SetContainer::getNodeSet(const std::string & nset) const
{
  const auto it = _node_set.find(MooseUtils::toUpper(nset));
  if (it == _node_set.end())
    throw std::runtime_error("Node set '" + nset + "' does not exist.");
  return it->second;
}

const std::vector<std::size_t> &
SetContainer::getElementSet(const std::string & elset) const
{
  const auto it = _element_set.find(MooseUtils::toUpper(elset));
  if (it == _element_set.end())
    throw std::runtime_error("Element set '" + elset + "' does not exist.");
  return it->second;
}

void
SetContainer::processSetHelper(std::map<std::string, std::vector<std::size_t>> & set_map,
                               const OptionNode & option,
                               const std::string & name_key)
{
  // parse the header line
  const auto & map = option._header;
  const auto name = map.get<std::string>(name_key);
  if (set_map.count(name))
    throw std::runtime_error("Repeated " + name_key + " declaration for '" + name +
                             "' in Abaqus input.");

  // implement GENERATE keyword
  const auto generate = map.get<bool>("generate");

  if (map.has("unsorted"))
    throw std::runtime_error("The UNSORTED keyword is not supported.");

  auto & set = set_map[name];
  std::set<std::size_t> unique_items(set.begin(), set.end());
  for (const auto & data : option._data)
  {
    if (generate)
    {
      // syntax check
      if (data.size() != 3)
        throw std::runtime_error("Expected three values in " + name_key + " definition '" + name +
                                 "' with GENERATE keyword in Abaqus input.");

      // generate range
      const auto begin = MooseUtils::convert<std::size_t>(data[0]);
      const auto end = MooseUtils::convert<std::size_t>(data[1]);
      const auto step = MooseUtils::convert<std::size_t>(data[2]);
      if (step == 0)
        throw std::runtime_error("Zero step in generated set.");
      for (std::size_t item = begin; item <= end; item += step)
        unique_items.insert(item - 1);
    }
    else
    {
      for (const auto i : index_range(data))
      {
        // check for existing set first
        const auto it = set_map.find(data[i]);
        if (it != set_map.end())
        {
          // insert existing set
          unique_items.insert(it->second.begin(), it->second.end());
        }
        else
        {
          const auto item = MooseUtils::convert<std::size_t>(data[i]);
          if (item > 0)
            unique_items.insert(item - 1);
          else
            throw std::runtime_error("Invalid ID in " + name_key + " '" + name +
                                     "' in Abaqus input.");
        }
      }
    }
  }

  set.assign(unique_items.begin(), unique_items.end());
}

// Entry point for the final parsing stage
void
process::Root(const BlockNode & root)
{
  _libmesh_id = 0;
  _mesh = buildTypedMesh<ReplicatedMesh>(3);

  auto block_func = [&](const std::string & key, const BlockNode & block)
  {
    if (key == "part")
    {
      const auto name = block._header.get<std::string>("name");
      _part.add(name, block);
    }

    if (key == "assembly")
    {
      const auto name = block._header.get<std::string>("name");
      if (_assembly.size() > 0)
        throw std::runtime_error("Only one Assembly per model is supported");
      _assembly.add(name, block, *this);
    }
  };

  root.forAll(nullptr, block_func);

  // we can have Part data at the root level. In that case we automatically generate
  // an assembly and an instance to instantiate the part. Note that this is happens OUT
  // OF ORDER and might cause a problem if assembly based models mix elsets/nsets with
  // a root_part model. :/
  Part root_part(root);
  if (!root_parts.element.empty * ())
  {
    if (_assembly.size() > 0)
      throw std::runtime_error(
          "You cannot mix part/assembly/instance based syntax with flat input syntax");

    const auto part_id = _part.add(root_part);
    Instance root_instance(part_id);
    // add assembly and add instance to assembly
    throw std::runtime_error("Assembly-less models are not yet supported");
  }
}

Instance::Instance(const OptionNode & option, const Root & root)
{
  const auto & data = option._data;
  const auto & part = root._part[option._header.get<std::string>("part")];

  RealVectorValue translation(0, 0, 0);
  RealTensorValue rotation(1, 0, 0, 0, 1, 0, 0, 0, 1);

  // translation
  if (data.size() > 0)
  {
    const auto col = vecTo<Real>(data);
    if (col.size() > 3)
      throw std::runtime_error("Too many values in instance translation data line");
    for (const auto i : col)
      translation(i) = col[i];
  }

  // rotation
  if (data.size() > 1)
  {
    const auto col = vecTo<Real>(data);
    if (col.size() != 7)
      throw std::runtime_error("Invalid number of values in instance rotation data line");

    // axis of rotation (normalized)
    RealVectorValue a(col[0], col[1], col[2]);
    RealVectorValue b(col[3], col[4], col[5]);
    auto n = b - a;
    const auto norm = n.norm();
    if (norm == 0)
      throw std::runtime_error("Zero-length rotation axis");
    n /= norm;

    // angle
    Real theta = col[6] / 180.0 * libMesh::pi;

    // rotation matrix
    const Real c = std::cos(theta);
    const Real s = std::sin(theta);
    const Real oc = 1.0 - c;
    rotation = RealVectorValue(c + ux * ux * oc,
                               ux * uy * oc - uz * s,
                               ux * uz * oc + uy * s,

                               uy * ux * oc + uz * s,
                               c + uy * uy * oc,
                               uy * uz * oc - ux * s,

                               uz * ux * oc - uy * s,
                               uz * uy * oc + ux * s,
                               c + uz * uz * oc, );
  }

  // generate mesh points
  for (const auto & [abaqus_node_id, pp] : part._node)
  {
    // transform part point pp to instance point ip
    Point ip = rotation * (pp + translation);

    // add the mesh point
    _libmesh_node_id[abaqus_node_id] = root._mesh_points.size();
    root._mesh_points.emplace_back(ip, 0);
  }

  // add elements
  const auto elem_id  = _elements.size();
  for (const auto & [abaqus_elem_id, abaqus_elem] : part._element)
  {
    const auto & uel = part._element_definition[abaqus_elem._type_id];
    LibMeshUElement elem(uel);
    // translate abaqus node ids to libmesh node ids
    for (const auto i : index_range(abaqus_elem._node_list))
    {
      const auto abaqus_node_id = abaqus_elem._node_list[i];
      const auto & var_mask = uel._var_mask[i];

      // get index into mesh point list
      const auto node_id = _libmesh_node_id[abaqus_node_id];

      // add global node id to element
      elem._libmesh_node_list.push_back(node_id);

      // build node to elem map
      _root._node_to_uel_map[node_id].push_back(elem_id);

      // update dof mask at global mesh point
      root._mesh_points[node_id].second |= var_mask;
    }

    _moose_elem_id[abaqus_elem_id] = elem_id;
    _elements.push_back(elem);
  }
}

Assembly::Assembly(const BlockNode & block, const Root & root) : SetContainer(nullptr)
{
  auto option_func = [&](const std::string & key, const OptionNode & option)
  {
    if (key == "instance")
    {
      const auto name = option._header.get<std::string>("name");
      _instance.add(name, option, root);
    }

    if (key == "elset")
    {
      const auto & instance_name = option._header.get<std::string>("instance", "");
      if (instance_name.empty())
        processElementSet(option);
      else
      {
        auto it = _instance.find(instance_name);
        if (it == _instance.end())
          throw std::runtime_error("Instance '" + instance_name +
                                   "' not found in elset declaration.");
        it->second.processElementSet(option);
      }
    }

    if (key == "nset")
    {
      const auto & instance_name = option._header.get<std::string>("instance", "");
      if (instance_name.empty())
        processNodeSet(option);
      else
      {
        auto it = _instance.find(instance_name);
        if (it == _instance.end())
          throw std::runtime_error("Instance '" + instance_name +
                                   "' not found in elset declaration.");
        it->second.processNodeSet(option);
      }
    }
  };

  root.forAll(option_func, nullptr);
}

} // namespace Abaqus
