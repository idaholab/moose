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

  // parse variable (DOF) info
  _vars.resize(_n_nodes);
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
    for (const auto i : index_range(col))
      // note how abaqus treats the first data line differently from the following lines!
      if (i > 0 || !line)
        var.push_back(col[i] - 1);
  }

  // check if any node numbers were omitted, and if so, set them to the preceeding node's variables
  for (const auto i : make_range(_n_nodes - 1))
    if (!seen_node.count(i + 1))
      for (const auto v : _vars[i])
        _vars[i + 1].push_back(v);
}

Part::Part(const BlockNode & block) : SetContainer(this)
{
  // User element definitions
  block.forOption("user element",
                  [&](const OptionNode & option)
                  {
                    const auto type = block._header.get<std::string>("type");
                    const auto id = _element_definition.add(type, block);
                    _element_definition[id]._type_id = id;
                  });

  // Nodes
  block.forOption(
      "node",
      [&](const OptionNode & option)
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
      });

  // Elements
  block.forOption(
      "element",
      [&](const OptionNode & option)
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
      });

  // part sets
  block.forOption("elset", [&](const OptionNode & option) { processElementSet(option); });
  block.forOption("nset", [&](const OptionNode & option) { processNodeSet(option); });
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
Root::process(const BlockNode & root)
{
  // Parts
  root.forBlock("part",
                [&](const BlockNode & block)
                {
                  const auto name = block._header.get<std::string>("name");
                  _part.add(name, block);
                });

  // Assembly
  root.forBlock("assembly",
                [&](const BlockNode & block)
                {
                  const auto name = block._header.get<std::string>("name");
                  _assembly.add(name, block, *this);
                });

  // we can have Part data at the root level. In that cae we automatically generate an assembly and
  // an instance to instantiate the part
  // Part root_part(root); // TODO
}

Instance::Instance(const OptionNode & option, const Root & root)
{
  const auto & data = option._data;
  _part_id = root._part.id(option._header.get<std::string>("part"));

  // translation
  if (data.size() > 0)
  {
    const auto col = vecTo<Real>(data);
    if (col.size() > 3)
      throw std::runtime_error("Too many values in instance translation data line");
    for (const auto i : col)
      _translation(i) = col[i];
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
    _rotation = RealVectorValue(c + ux * ux * oc,
                                ux * uy * oc - uz * s,
                                ux * uz * oc + uy * s,

                                uy * ux * oc + uz * s,
                                c + uy * uy * oc,
                                uy * uz * oc - ux * s,

                                uz * ux * oc - uy * s,
                                uz * uy * oc + ux * s,
                                c + uz * uz * oc, );
  }
  else
    // no rotation - set to identity
    _rotation = RealVectorValue(1, 0, 0, 0, 1, 0, 0, 0, 1);
}

Assembly::Assembly(const BlockNode & block, const Root & root) : SetContainer(nullptr)
{
  // Instance
  block.forOption("instance",
                  [&](const OptionNode & option)
                  {
                    const auto name = block._header.get<std::string>("name");
                    _instance.add(name, block, root);
                  });

  // assembly sets (might be instance specific)
  block.forOption("elset",
                  [&](const OptionNode & option)
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
                  });
  block.forOption("nset",
                  [&](const OptionNode & option)
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
                  });
}

} // namespace Abaqus
