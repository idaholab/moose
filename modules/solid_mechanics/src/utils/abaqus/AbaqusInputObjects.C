//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusInputObjects.h"
#include "MooseError.h"
#include "MooseUtils.h"
#include "libmesh/libmesh_common.h"

#include <iostream>
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

// do not complain when trying to convert floating point numbers to int
std::vector<int>
vecToInt(const std::vector<std::string> & in)
{
  std::vector<int> out;
  std::transform(in.begin(),
                 in.end(),
                 std::back_inserter(out),
                 [](const std::string & str) { return std::stoi(str); });
  return out;
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

  // parse type string (should be UX with X being an integer from 1 to 9,999)
  if (_type.empty() || (_type[0] != 'U' && _type[0] != 'u'))
    mooseError("Invalid type string in UEL definition: ", _type);
  _type_id = std::stoi(_type.substr(1));
  if (_type_id < 1 || _type_id > 9999)
    mooseError("Invalid type string in UEL definition: ", _type);

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
      mooseError("Invalid node number in Abaqus input.");

    // mark off node as seen and check for duplicates
    if (seen_node.count(node_number))
      mooseError("Duplicate node in '*user element' section.");
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
          mooseError("Currently variables numbers >= ", bits, " are not supported.");
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

void
Part::parse(const BlockNode & block)
{
  auto option_func = [this](const std::string & key, const OptionNode & option)
  { Part::optionFunc(key, option); };
  block.forAll(option_func, nullptr);
}

void
Part::optionFunc(const std::string & key, const OptionNode & option)
{
  std::cout << "Part::optionFunc " << key << "\n";
  // User element definitions
  if (key == "user element")
  {
    const auto type = option._header.get<std::string>("type");
    _element_definition.add(type, option);
  }

  // Nodes
  if (key == "node")
  {
    const auto & map = option._header;
    if (map.get<std::string>("system", "R") != "R")
      mooseError("Only cartesian coordinates are currently supported");

    // this should not be hard to add though
    if (map.has("input"))
      mooseError("External coordinate inputs are not yet supported");

    auto * nset = map.has("nset") ? &_nsets[map.get<std::string>("nset")] : nullptr;
    std::set<Index> unique_ids;
    if (nset)
      unique_ids.insert(nset->begin(), nset->end());

    // loop over data lines
    for (const auto & data : option._data) // or loop over external input lines
    {
      if (data.size() < 2)
        mooseError("Insufficient data in node data line");
      if (data.size() > 4)
        mooseError("Normal directions are not yet supported");

      const auto id = MooseUtils::convert<AbaqusID>(data[0]);
      Point p(MooseUtils::convert<Real>(data[1]),
              data.size() > 2 ? MooseUtils::convert<Real>(data[2]) : 0,
              data.size() > 3 ? MooseUtils::convert<Real>(data[3]) : 0);

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
    auto * elset = map.has("elset") ? &_elsets[map.get<std::string>("elset")] : nullptr;
    std::set<std::size_t> unique_ids;
    if (elset)
      unique_ids.insert(elset->begin(), elset->end());

    // loop over data lines
    for (const auto & data : option._data)
    {
      const auto col = vecTo<AbaqusID>(data);

      // check number of nodes
      if (col.size() - 1 != uel._n_nodes)
        mooseError("Wrong number of nodes for user element of type '", type, "' in Abaqus input.");

      // prepare empty element
      const auto id = col[0];
      Element element{id, uel, {}, {nullptr, nullptr}};

      // copy in node numbers (converting from 1-base to 0-base)
      for (const auto i : index_range(col))
        if (i > 0)
        {
          const auto node_index = _node_id_to_index.at(col[i]);
          element._nodes.push_back(node_index);
          _nodes[node_index]._var_mask |= uel._var_mask[i - 1];
        }

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
    const auto & elset = _elsets.at(map.get<std::string>("elset"));

    // locate space
    _properties.emplace_back();
    auto & props = _properties.back();

    for (const auto & data : option._data)
    {
      const auto dprop = vecTo<Real>(data);
      const auto iprop = vecToInt(data);
      props.first.insert(props.first.end(), dprop.begin(), dprop.end());
      props.second.insert(props.second.end(), iprop.begin(), iprop.end());
    }

    // assign properties to elements
    for (const auto index : elset)
    {
      auto & elem = _elements[index];
      const auto & uel = elem._uel;
      if (uel._n_properties > 0)
        elem._properties.first = props.first.data();
      else
        elem._properties.first = nullptr;

      if (uel._n_iproperties > 0)
      {
        mooseError("integer properties not supported yet (fix conversion to skip floats in data)");
        elem._properties.second = &(props.second[uel._n_properties]);
      }
      else
        elem._properties.second = nullptr;
    }
  }
}

void
Part::processNodeSet(const OptionNode & option, Instance * instance)
{
  const auto & map = option._header;
  const Index offset = instance ? instance->_local_to_global_node_index_offset : 0;

  // copy nodes from element set
  if (map.has("elset"))
  {
    const auto & source_part = instance ? instance->_part : *this;

    // get element set to copy nodes from
    const auto & elset = source_part._elsets.at(map.get<std::string>("elset"));

    // get or create node set
    auto & nset = _nsets[map.get<std::string>("nset")];

    // make set unique and copy nodes into it
    std::set<Index> unique_nodes(nset.begin(), nset.end());
    for (const auto elem_index : elset)
      for (const auto node_index : _elements[elem_index]._nodes)
        unique_nodes.insert(node_index + offset);
    nset.assign(unique_nodes.begin(), unique_nodes.end());
  }
  else
    // data lines are only present if elset parameter is _not_ specified
    processSetHelper<true>(option, instance);
}

void
Part::processElementSet(const OptionNode & option, Instance * instance)
{
  processSetHelper<false>(option, instance);
}

template <bool is_nodal>
void
Part::processSetHelper(const OptionNode & option, Instance * instance)
{
  const auto & id_to_index = is_nodal ? _node_id_to_index : _element_id_to_index;
  const auto & name_key = is_nodal ? "nset" : "elset";
  auto & set_map = is_nodal ? _nsets : _elsets;

  // parse the header line
  const auto & map = option._header;
  const auto name = map.get<std::string>(name_key);
  const auto offset = instance ? (is_nodal ? instance->_local_to_global_node_index_offset
                                           : instance->_local_to_global_element_index_offset)
                               : 0.0;

  std::cout << "Processing " << name_key << ' ' << name << std::endl;

  // implement GENERATE keyword
  const auto generate = map.get<bool>("generate");

  if (map.has("unsorted"))
    mooseError("The UNSORTED keyword is not supported.");

  // get or create the set
  auto & set = set_map[name];
  std::set<std::size_t> unique_items(set.begin(), set.end());
  for (const auto & data : option._data)
  {
    if (generate)
    {
      // syntax check
      if (data.size() != 3)
        mooseError("Expected three values in ",
                   name_key,
                   " definition '",
                   name,
                   "' with GENERATE keyword in Abaqus input.");

      // generate range
      const auto begin = MooseUtils::convert<AbaqusID>(data[0]);
      const auto end = MooseUtils::convert<AbaqusID>(data[1]);
      const auto step = MooseUtils::convert<AbaqusID>(data[2]);
      if (step == 0)
        mooseError("Zero step in generated set.");
      for (AbaqusID item = begin; item <= end; item += step)
        unique_items.insert(id_to_index.at(item) + offset);
    }
    else
    {
      // TODO: here we need to to implement instance.node_id syntax!
      // TODO: we'll need the root model here!!! Assembly->Instance->Part
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

Instance::Instance(const BlockNode & block, AssemblyModel & model)
  : _part(model._part[block._header.get<std::string>("part")])
{
  // const auto & data = block._data;

  RealVectorValue translation(0, 0, 0);
  RealTensorValue rotation(1, 0, 0, 0, 1, 0, 0, 0, 1);

  /*
  // translation
  if (data.size() > 0)
  {
    const auto col = vecTo<Real>(data[0]);
    if (col.size() > 3)
      mooseError("Too many values in instance translation data line");
    for (const auto i : col)
      translation(i) = col[i];
  }

  // rotation
  if (data.size() > 1)
  {
    const auto col = vecTo<Real>(data[1]);
    if (col.size() != 7)
      mooseError("Invalid number of values in instance rotation data line");

    // axis of rotation (normalized)
    RealVectorValue a(col[0], col[1], col[2]);
    RealVectorValue b(col[3], col[4], col[5]);
    auto n = b - a;
    const auto norm = n.norm();
    if (norm == 0)
      mooseError("Zero-length rotation axis");
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
*/

  // TODO: block can have _data lines, too!!! :-O

  // store offsets
  _local_to_global_node_index_offset = model._nodes.size();
  _local_to_global_element_index_offset = model._elements.size();

  // instantiate mesh points
  for (const auto & part_node : _part._nodes)
  {
    // make a copy of the _part node
    auto model_node = part_node;

    // transform instantiated node
    model_node._point = rotation * (model_node._point + translation);

    // add the mesh point to the model level list
    model._nodes.push_back(model_node);
  }

  // instantiate elements
  for (const auto & part_element : _part._elements)
  {
    // make a copy of the part element
    auto model_element = part_element;

    // shift part node indices to model node indices
    for (auto & index : model_element._nodes)
      index += _local_to_global_node_index_offset;

    model._elements.push_back(model_element);
  }

  // merge uel definitions
  model._element_definition.merge(_part._element_definition);

  // apply node sets
  for (const auto & [nset_name, nset] : _part._nsets)
  {
    auto & model_nset = model._nsets[nset_name];
    for (const auto & index : nset)
      model_nset.push_back(index + _local_to_global_node_index_offset);
  }

  // apply element sets
  for (const auto & [elset_name, elset] : _part._elsets)
  {
    auto & model_elset = model._elsets[elset_name];
    for (const auto & index : elset)
      model_elset.push_back(index + _local_to_global_element_index_offset);
  }
}

void
Assembly::parse(const BlockNode & block, AssemblyModel & model)
{
  auto option_func = [&](const std::string & key, const OptionNode & option)
  {
    if (key == "elset")
    {
      const auto & instance_name = option._header.get<std::string>("instance", "");
      if (instance_name.empty())
      {
        model.processElementSet(option); // What do we do here????? Iterate over all instances?
      }
      else
      {
        if (!_instance.has(instance_name))
          mooseError("Instance '", instance_name, "' not found in elset declaration.");
        model.processElementSet(option, &_instance[instance_name]);
      }
    }

    if (key == "nset")
    {
      const auto & instance_name = option._header.get<std::string>("instance", "");
      if (instance_name.empty())
      {
        model.processNodeSet(option); // What do we do here????? Iterate over all instances?
      }
      else
      {
        if (!_instance.has(instance_name))
          mooseError("Instance '", instance_name, "' not found in nset declaration.");
        model.processNodeSet(option, &_instance[instance_name]);
      }
    }
  };

  auto block_func = [&](const std::string & key, const BlockNode & block)
  {
    if (key == "instance")
    {
      const auto name = block._header.get<std::string>("name");
      std::cout << "Adding instance " << name << std::endl;
      _instance.add(name, block, model);
    }
  };

  block.forAll(option_func, block_func);
}

FieldIC::FieldIC(const OptionNode & option, const Model & model)
{
  _var = option._header.get<int>("variable") - 1;
  if (_var < 0)
    mooseError("invalid variable number ", _var + 1);

  // loop over data lines
  for (const auto & data : option._data)
  {
    if (data.size() != 2)
      mooseError("Expected two items in data line for field IC");
    const auto name = data[0];
    const auto value = MooseUtils::convert<Real>(data[1]);

    // freeze state of the node set
    if (_nsets.find(name) == _nsets.end())
      _nsets[name] = model._nsets.at(name);

    // add node set name and initial value to the list
    _value.emplace_back(name, value);
  }
}

Boundary::Boundary(const OptionNode & /*option*/, Model & /*model*/)
{
  // get the current state of the node set
}

void
Model::optionFunc(const std::string & key, const OptionNode & option)
{
  if (key == "initial conditions")
  {
    const auto type = option._header.get<std::string>("type");
    if (MooseUtils::toLower(type) == "field")
      _field_ics.emplace_back(option, *this);
    else
      mooseError("Unsupported IC type");
  }
}

// Entry point for the final parsing stage
void
FlatModel::parse(const BlockNode & root)
{
  auto option_func = [this](const std::string & key, const OptionNode & option)
  {
    Part::optionFunc(key, option);
    Model::optionFunc(key, option);
  };

  root.forAll(option_func, nullptr);
}

// Entry point for the final parsing stage
void
AssemblyModel::parse(const BlockNode & root)
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
        mooseError("Only one Assembly per model is supported");
      _assembly = std::make_unique<Assembly>();
      _assembly->parse(block, *this);
    }

    else if (key == "step")
    {
    }

    else
      mooseError("Unsupported block found in input: ", key);
  };

  auto option_func = [this](const std::string & key, const OptionNode & option)
  { Model::optionFunc(key, option); };

  root.forAll(option_func, block_func);
}

} // namespace Abaqus
