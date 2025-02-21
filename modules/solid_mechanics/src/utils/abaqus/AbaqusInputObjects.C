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

#include <memory>
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

Part::parse(const BlockNode & block) : SetContainer(this)
{
  auto option_func = [this](const std::string & key, const OptionNode & option)
  { Part::optionFunc(key, option); };
  block.forAll(option_func, nullptr);
}

void
Part::optionFunc(const std::string & key, const OptionNode & option)
{
  auto option_func = [&](const std::string & key, const OptionNode & option)
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

      auto * nset = map.has("nset") ? &_nsets[map.get<std::string>("nset")] : nullptr;
      std::set<Index> unique_ids;
      if (nset)
        unique_ids.insert(nset->begin(), nset->end());

      // loop over data lines
      for (const auto & data : option._data) // or loop over external input lines
      {
        if (data.size() < 4)
          throw std::runtime_error("Insufficient data in node data line");
        if (data.size() > 4)
          throw std::runtime_error("Normal directions are not yet supported");

        const auto id = MooseUtils::convert<AbaqusID>(data[0]);
        Point p(MooseUtils::convert<Real>(data[1]),
                MooseUtils::convert<Real>(data[2]),
                MooseUtils::convert<Real>(data[3]));

        Index index = _nodes.size();
        _nodes.push_back(Node{id, p, 0});
        _node_id_to_index[id] = index;

        if (nset)
          unique_ids.insert(index);
      }

      if (nset)
        nset->assign(unique_ids.begin(), unique_ids.end());
    }

    // Elements
    if (key == "element")
    {
      const auto & map = option._header;
      const auto type = map.get<std::string>("type");
      const UserElement & uel = _element_definition[type];

      // add new elements to an element set?
      auto * elset = map.has("elset") ? &_element_set[map.get<std::string>("elset")] : nullptr;
      std::set<std::size_t> unique_ids;
      if (elset)
        unique_ids.insert(elset->begin(), elset->end());

      // loop over data lines
      for (const auto & data : option._data)
      {
        const auto col = vecTo<AbaqusID>(data);

        // check number of nodes
        if (col.size() - 1 != uel._n_nodes)
          throw std::runtime_error("Wrong number of nodes for user element of type '" + type +
                                   "' in Abaqus input.");

        // prepare empty element
        const auto id = col[0];
        Element element{id, uel, {}, {nullptr, nullptr}};

        // copy in node numbers (converting from 1-base to 0-base)
        for (const auto i : index_range(col))
          if (i > 0)
            element._nodes.push_back(_node_id_to_index.at(col[i]));

        // insert into unordered map
        Index index = _elements.size();
        _elements.push_back(element);
        _element_id_to_index[id] = index;

        if (elset)
          unique_ids.insert(index);
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
}

void
Part::processNodeSet(const OptionNode & option)
{
  const auto & map = option._header;

  // copy nodes from element set
  if (map.has("elset"))
  {
    // get element set to copy nodes from
    const auto & elset = _elsets.at(map.get<std::string>("elset"));

    // get or create node set
    auto & nset = _node_set[map.get<std::string>("nset")];

    // make set unique and copy nodes into it
    std::set<Index> unique_nodes(nset.begin(), nset.end());
    for (const auto elem_index : elset)
      for (const auto node_index : _elements[elem_index]._nodes)
        unique_nodes.insert(node_index);
    nset.assign(unique_nodes.begin(), unique_nodes.end());
  }
  else
    // data lines are only present if elset parameter is _not_ specified
    processSetHelper(_nsets, _node_id_to_index, option, "nset");
}

void
Part::processElementSet(const OptionNode & option)
{
  processSetHelper(_elsets, _element_id_to_index, option, "elset");
}

void
Part::processSetHelper(std::map<std::string, std::vector<Index>> & set_map,
                       std::unordered_map<AbaqusID, Index> & id_to_index,
                       const OptionNode & option,
                       const std::string & name_key)
{
  // parse the header line
  const auto & map = option._header;
  const auto name = map.get<std::string>(name_key);

  // implement GENERATE keyword
  const auto generate = map.get<bool>("generate");

  if (map.has("unsorted"))
    throw std::runtime_error("The UNSORTED keyword is not supported.");

  // get or create the set
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
      const auto begin = MooseUtils::convert<AbaqusID>(data[0]);
      const auto end = MooseUtils::convert<AbaqusID>(data[1]);
      const auto step = MooseUtils::convert<AbaqusID>(data[2]);
      if (step == 0)
        throw std::runtime_error("Zero step in generated set.");
      for (AbaqusID item = begin; item <= end; item += step)
        unique_items.insert(id_to_index.at(item));
    }
    else
    {
      // TODO: here we need to to implement instance.node_id syntax!
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
          const auto item = MooseUtils::convert<AbaqusID>(data[i]);
          unique_items.insert(id_to_index.at(item));
        }
      }
    }
  }

  set.assign(unique_items.begin(), unique_items.end());
}

Instance::Instance(const OptionNode & option, AssemblyRoot & root)
{
  const auto & data = option._data;
  const auto & part = root._part[option._header.get<std::string>("part")];

  RealVectorValue translation(0, 0, 0);
  RealTensorValue rotation(1, 0, 0, 0, 1, 0, 0, 0, 1);

  // translation
  if (data.size() > 0)
  {
    const auto col = vecTo<Real>(data[0]);
    if (col.size() > 3)
      throw std::runtime_error("Too many values in instance translation data line");
    for (const auto i : col)
      translation(i) = col[i];
  }

  // rotation
  if (data.size() > 1)
  {
    const auto col = vecTo<Real>(data[1]);
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
    rotation = RealTensorValue(c + n(0) * n(0) * oc,
                               n(0) * n(1) * oc - n(2) * s,
                               n(0) * n(2) * oc + n(1) * s,

                               n(1) * n(0) * oc + n(2) * s,
                               c + n(1) * n(1) * oc,
                               n(1) * n(2) * oc - n(0) * s,

                               n(2) * n(0) * oc - n(1) * s,
                               n(2) * n(1) * oc + n(0) * s,
                               c + n(2) * n(2) * oc);
  }

  // store offsets
  _local_to_global_node_index_offset = root._nodes.size();
  _local_to_global_element_index_offset = root._elements.size();

  // instantiate mesh points
  for (const auto & part_node : part._nodes)
  {
    // make a copy of the part node
    auto root_node = part_node;

    // transform instantiated node
    root_node._point = rotation * (root_node._point + translation);

    // add the mesh point to the root level list
    root._nodes.push_back(root_node);
  }

  // instantiate elements
  for (const auto & part_element : part._elements)
  {
    // make a copy of the part element
    auto root_element = part_element;

    // shift part node indices to root node indices
    for (auto & index : root_element._nodes)
      index += _local_to_global_node_index_offset;

    root._elements.push_back(root_element);
  }

  // apply node sets
  for (const auto & [nset_name, nset] : part._nsets)
}

Assembly::Assembly(const BlockNode & block, AssemblyRoot & root) : SetContainer(nullptr)
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
      {
      } //  processElementSet(option); // What do we do here????? Iterate over all instances?
      else
      {
        if (!_instance.has(instance_name))
          throw std::runtime_error("Instance '" + instance_name +
                                   "' not found in elset declaration.");
        //_instance[instance_name].processElementSet(option);
        // .. and here?
      }
    }

    if (key == "nset")
    {
      const auto & instance_name = option._header.get<std::string>("instance", "");
      if (instance_name.empty())
      {
      } //   processNodeSet(option); // What do we do here????? Iterate over all instances?
      else
      {
        if (!_instance.has(instance_name))
          throw std::runtime_error("Instance '" + instance_name +
                                   "' not found in nset declaration.");
        //_instance[instance_name].processNodeSet(option);
        // .. and here?
        // look up the instance offset and the part node set and add the listed nodes via
        // _node_id_to_index[listed_id]+offset to root
      }
    }
  };

  root.forAll(option_func, nullptr);
}

// Entry point for the final parsing stage
void
FlatRoot::parse(const BlockNode & root)
{
  auto option_func = [this](const std::string & key, const OptionNode & option)
  { Part::optionFunc(key, option); };

  root.forAll(option_func, nullptr);
}

// Entry point for the final parsing stage
void
AssemblyRoot::parse(const BlockNode & root)
{
  auto block_func = [&](const std::string & key, const BlockNode & block)
  {
    if (key == "part")
    {
      const auto name = block._header.get<std::string>("name");
      _part.add(name, block);
    }

    else if (key == "assembly")
    {
      // const auto name = block._header.get<std::string>("name");
      if (_assembly)
        throw std::runtime_error("Only one Assembly per model is supported");
      _assembly = std::make_unique<Assembly>();
      _assembly->parse(block, *this);
    }

    else if (key == "step")
    {
    }

    else
      throw std::runtime_error("Unsupported block found in input: " + key);
  };

  root.forAll(nullptr, block_func);
}

Boundary::Boundary(const OptionNode & option, Root & root)
{
  // get the current state of the node set
}

} // namespace Abaqus
