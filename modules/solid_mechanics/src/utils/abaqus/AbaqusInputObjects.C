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
#include "MooseStringUtils.h"
#include "MooseUtils.h"
#include "libmesh/libmesh_common.h"

#include <iostream>
#include <limits>
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

    // copy in var numbers
    auto & var = _vars[node_number];
    auto & var_mask = _var_mask[node_number];
    for (const auto i : index_range(col))
      // note how abaqus treats the first data line differently from the following lines!
      if (i > 0 || !line)
      {
        const auto v = col[i];
        if (v > bits)
          mooseError("Currently variables numbers >= ", bits, " are not supported.");
        if (v < 1)
          mooseError("Invalid variable number in Abaqus input.");

        var.push_back(v);
        var_mask |= (1 << (v - 1));
      }
  }

  // check if any node numbers were omitted, and if so, set them to the preceeding node's variables
  for (const auto i : make_range(_n_nodes - 1))
    if (!seen_node.count(i + 1))
    {
      _vars[i + 1] = _vars[i];
      _var_mask[i + 1] = _var_mask[i];
    }
}

void
Part::parse(const BlockNode & block)
{
  auto option_func = [this](const std::string & key, const OptionNode & option)
  {
    if (!Part::optionFunc(key, option))
      mooseError("Unsupported option ", key);
  };
  block.forAll(option_func, nullptr);
}

bool
Part::optionFunc(const std::string & key, const OptionNode & option)
{
  // User element definitions
  if (key == "user element")
  {
    const auto type = option._header.get<std::string>("type");
    _element_definition.add(type, option);
  }

  // Nodes
  else if (key == "node")
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
  else if (key == "element")
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

  else if (key == "elset")
    processElementSet(option, nullptr, nullptr);
  else if (key == "nset")
    processNodeSet(option, nullptr, nullptr);

  // UEL properties
  else if (key == "uel property")
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
  else
    return false;
  return true;
}

void
Part::processNodeSet(const OptionNode & option, Instance * instance, const AssemblyModel * asmb)
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
    {
      const auto & elem = instance ? instance->_part._elements[elem_index] : _elements[elem_index];
      for (const auto node_index : elem._nodes)
        unique_nodes.insert(node_index + offset);
    }
    nset.assign(unique_nodes.begin(), unique_nodes.end());
  }
  else
    // data lines are only present if elset parameter is _not_ specified
    processSetHelper<true>(option, instance, asmb);
}

void
Part::processElementSet(const OptionNode & option, Instance * instance, const AssemblyModel * asmb)
{
  // Use the generic set helper for element sets as well.
  // Assembly-level elsets with numeric-only entries should normally be instance-scoped; tests
  // prefer inline instance-qualified tokens (e.g., I2.1) to avoid ambiguity.
  processSetHelper<false>(option, instance, asmb);
}

template <bool is_nodal>
void
Part::processSetHelper(const OptionNode & option, Instance * instance, const AssemblyModel * asmb)
{
  const auto & index_host = instance ? instance->_part : *this;
  const auto & id_to_index =
      is_nodal ? index_host._node_id_to_index : index_host._element_id_to_index;
  const auto & name_key = is_nodal ? "nset" : "elset";
  auto & set_map = is_nodal ? _nsets : _elsets;

  // parse the header line
  const auto & map = option._header;
  const auto name = map.get<std::string>(name_key);
  const auto offset = instance ? (is_nodal ? instance->_local_to_global_node_index_offset
                                           : instance->_local_to_global_element_index_offset)
                               : 0;

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
      // For assembly-level nodal sets, require an instance (either header or inline); GENERATE
      // without instance context is not supported.
      if constexpr (is_nodal)
        if (asmb && !instance)
          mooseError("Assembly-level *Nset with GENERATE requires an instance (header or inline).");
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
      // Support inline entries that can be:
      // - another set name (already built in set_map)
      // - a plain numeric id (interpreted in the provided instance or this Part)
      // - an instance-qualified id: "<instance>.<id>" (requires header instance or single instance)

      for (const auto i : index_range(data))
      {
        const auto & atom = data[i];

        // 1) Referencing an existing set by name
        const auto it = set_map.find(atom);
        if (it != set_map.end())
        {
          unique_items.insert(it->second.begin(), it->second.end());
          continue;
        }

        // 2) Instance-qualified id: "instanceName.id"
        std::vector<std::string> parts;
        MooseUtils::tokenize(atom, parts, 1, ".");
        if (parts.size() == 2)
        {
          // Either inline instance OR header instance (mutually exclusive)
          if (instance)
            mooseError("Ambiguous set token '",
                       atom,
                       "': both inline instance and header instance are provided.");

          if (!asmb || !asmb->_assembly)
            mooseError("Instance-qualified entry '", atom, "' requires Assembly context.");

          const auto & inst_name = parts[0];
          const auto id = MooseUtils::convert<AbaqusID>(parts[1]);
          if (!asmb->_assembly->_instance.has(inst_name))
            mooseError("Instance '", inst_name, "' not found while resolving '", atom, "'.");

          const auto & inst = asmb->_assembly->_instance[inst_name];
          const auto & local_map =
              is_nodal ? inst._part._node_id_to_index : inst._part._element_id_to_index;
          const auto inst_offset = is_nodal ? inst._local_to_global_node_index_offset
                                            : inst._local_to_global_element_index_offset;
          unique_items.insert(local_map.at(id) + inst_offset);
          continue;
        }

        // 3) Plain numeric id in the current scope
        if (asmb && !instance && is_nodal)
          mooseError(
              "Assembly-level *Nset requires an instance: use 'instance=' or inline 'inst.id'");
        const auto item = MooseUtils::convert<AbaqusID>(atom);
        unique_items.insert(id_to_index.at(item) + offset);
      }
    }
  }

  set.assign(unique_items.begin(), unique_items.end());
}

void
Step::parse(const BlockNode & block)
{
  _dt = -1.0;

  auto option_func = [this](const std::string & key, const OptionNode & option)
  {
    if (!Step::optionFunc(key, option))
      mooseError("Unsupported option ", key);
  };
  block.forAll(option_func, nullptr);

  if (_dt <= 0.0)
    mooseError("invalid or missing time step in *Step block");
}

bool
Step::optionFunc(const std::string & key, const OptionNode & option)
{
  // User element definitions
  if (key == "boundary")
  {
    // copy over BC data from previous step (unless this is a model level BC or OP=NEW)
    const auto op = MooseUtils::toLower(option._header.get<std::string>("op", "mod"));
    if (op != "mod" && op != "new")
      mooseError("Unknown value for *Boundary OP=", op);
    if (&_model != this && op == "mod")
    {
      const auto & previous_step =
          _model._step.size() > 0 ? _model._step[_model._step.size() - 1] : _model;
      _bc_var_node_value_map = previous_step._bc_var_node_value_map;
    }

    // loop over data lines
    for (const auto & data : option._data)
    {
      if (data.size() < 2)
        mooseError("At least two fields are required in each Boundary data line");

      // nodes this line applies to (either a nset or a single node)
      const std::vector<Index> * node_set_ptr;
      std::vector<Index> single_node;

      if (_model._nsets.find(data[0]) != _model._nsets.end())
        node_set_ptr = &_model._nsets.at(data[0]);
      else
      {
        // If the option is instance-scoped, resolve the single node within that instance
        const Instance * scope_instance = nullptr;
        if (option._header.has("instance"))
        {
          const auto inst_name = option._header.get<std::string>("instance");
          const auto * asmb = dynamic_cast<const AssemblyModel *>(&_model);
          if (!asmb || !asmb->_assembly)
            mooseError("*Boundary with instance= requires an Assembly context.");
          if (!asmb->_assembly->_instance.has(inst_name))
            mooseError("Instance '", inst_name, "' not found for *Boundary instance=");
          scope_instance = &asmb->_assembly->_instance[inst_name];
        }

        single_node = {_model.getNodeIndex(data[0], scope_instance)};
        node_set_ptr = &single_node;
      }

      // starting and ending Abaqus Variable (DOF) ID
      const auto first_dof_id = MooseUtils::convert<AbaqusID>(data[1]);
      const auto last_dof_id =
          data.size() > 2 ? MooseUtils::convert<AbaqusID>(data[2]) : first_dof_id;

      // the applied BC value defaults to 0

      const Real value = data.size() > 3 ? MooseUtils::convert<Real>(data[3]) : 0.0;

      // fill in data
      for (AbaqusID dof_id = first_dof_id; dof_id <= last_dof_id; ++dof_id)
      {
        auto & node_value = _bc_var_node_value_map[dof_id];
        for (const auto node_index : *node_set_ptr)
          node_value[node_index] = value;
      }
    }
  }
  else if (key == "static")
  {
    if (option._data.size() != 1)
      mooseError("Expected 1 data line for *Static option");
    const auto col = vecTo<Real>(option._data[0]);

    if (col.size() > 0)
      _dt = col[0];

    if (col.size() > 1)
      _duration = col[1];
    else
      _duration = 1.0;

    if (col.size() > 2)
      _dt_min = col[2];
    else
      _dt_min = 1e-5 * _duration;

    if (col.size() >= 3)
      _dt_max = col[3];
    else
      _dt_max = std::numeric_limits<Real>::max();
  }
  else if (key == "dload")
  {
    // TODO
  }
  else if (key == "restart")
  {
    // TODO
  }
  else if (key == "output")
  {
    // TODO
  }
  else
    return false;
  return true;
}

Instance::Instance(const BlockNode & block, AssemblyModel & model)
  : _part(model._part[block._header.get<std::string>("part")])
{
  // Multiple instances are supported. Numeric IDs at assembly scope must be instance-qualified
  // elsewhere to avoid ambiguity; code paths already enforce that where needed.

  RealVectorValue translation(0, 0, 0);
  RealTensorValue rotation(1, 0, 0, 0, 1, 0, 0, 0, 1);
  const auto & data = block._data;

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
        model.processElementSet(option, nullptr, &model); // model-level set
      }
      else
      {
        if (!_instance.has(instance_name))
          mooseError("Instance '", instance_name, "' not found in elset declaration.");
        model.processElementSet(option, &_instance[instance_name], &model);
      }
    }

    else if (key == "nset")
    {
      const auto & instance_name = option._header.get<std::string>("instance", "");
      if (instance_name.empty())
      {
        model.processNodeSet(option, nullptr, &model);
      }
      else
      {
        if (!_instance.has(instance_name))
          mooseError("Instance '", instance_name, "' not found in nset declaration.");
        model.processNodeSet(option, &_instance[instance_name], &model);
      }
    }
    else
      mooseError("Unsupported option ", key);
  };

  auto block_func = [&](const std::string & key, const BlockNode & block)
  {
    if (key == "instance")
    {
      const auto name = block._header.get<std::string>("name");
      _instance.add(name, block, model);
    }
    else
      mooseError("Unsupported block ", key);
  };

  block.forAll(option_func, block_func);
}

FieldIC::FieldIC(const OptionNode & option, const Model & model)
{
  _var = option._header.get<int>("variable");
  if (_var < 1)
    mooseError("invalid variable number ", _var);

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

bool
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
  else if (key == "heading")
  {
    // do nothing with this
  }
  else if (key == "preprint")
  {
    // do nothing with this
  }
  else
    return Step::optionFunc(key, option) || Part::optionFunc(key, option);
  return true;
}

bool
Model::blockFunc(const std::string & key, const BlockNode & block)
{
  if (key == "step")
  {
    const auto name = block._header.get<std::string>("name");
    _step.add(name, block, *this);
  }
  else
    return false;
  return true;
}

// Entry point for the final parsing stage
void
FlatModel::parse(const BlockNode & root)
{
  auto option_func = [this](const std::string & key, const OptionNode & option)
  {
    if (!Model::optionFunc(key, option))
      mooseError("Unsupported option ", key);
  };
  auto block_func = [&](const std::string & key, const BlockNode & block)
  {
    if (!Model::blockFunc(key, block))
      mooseError("Unsupported block ", key);
  };

  root.forAll(option_func, block_func);
}

Index
FlatModel::getNodeIndex(const std::string & key, const Instance * instance) const
{
  if (instance)
    mooseError("Instance-scoped lookup is not supported in FlatModel.");
  return _model._node_id_to_index.at(MooseUtils::convert<AbaqusID>(key));
}

Index
FlatModel::getElementIndex(const std::string & key, const Instance * instance) const
{
  if (instance)
    mooseError("Instance-scoped lookup is not supported in FlatModel.");
  return _model._element_id_to_index.at(MooseUtils::convert<AbaqusID>(key));
}

const Instance &
Model::getInstance(const std::string & /*name*/) const
{
  // FlatModel path (no instances): error
  mooseError("Instance lookup not supported in this model.");
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

    else if (!Model::blockFunc(key, block))
      mooseError("Unsupported block found in input: ", key);
  };

  auto option_func = [this](const std::string & key, const OptionNode & option)
  {
    if (!Model::optionFunc(key, option))
      mooseError("Unsupported option ", key);
  };

  root.forAll(option_func, block_func);
}

Index
AssemblyModel::getNodeIndex(const std::string & key, const Instance * instance) const
{
  // If caller provided an instance, enforce clean integer and resolve within it
  if (instance)
  {
    std::vector<std::string> parts;
    MooseUtils::tokenize(key, parts, 1, ".");
    if (parts.size() != 1)
      mooseError("Instance-scoped node lookup expects a clean integer id but got '", key, "'.");
    const auto node_id = MooseUtils::convert<AbaqusID>(key);
    const auto local_index = instance->_part._node_id_to_index.at(node_id);
    return local_index + instance->_local_to_global_node_index_offset;
  }

  // Otherwise support "instanceName.nodeId" or plain numeric ids
  std::vector<std::string> parts;
  MooseUtils::tokenize(key, parts, 1, ".");
  if (parts.size() == 2)
  {
    const auto & instance_name = parts[0];
    const auto node_id = MooseUtils::convert<AbaqusID>(parts[1]);
    if (!_assembly)
      mooseError("Node reference '", key, "' requires an Assembly but none was parsed.");
    if (!_assembly->_instance.has(instance_name))
      mooseError("Instance '", instance_name, "' not found while resolving node '", key, "'.");
    const auto & inst = _assembly->_instance[instance_name];
    const auto local_index = inst._part._node_id_to_index.at(node_id);
    return local_index + inst._local_to_global_node_index_offset;
  }
  return _model._node_id_to_index.at(MooseUtils::convert<AbaqusID>(key));
}

Index
AssemblyModel::getElementIndex(const std::string & key, const Instance * instance) const
{
  if (instance)
  {
    std::vector<std::string> parts;
    MooseUtils::tokenize(key, parts, 1, ".");
    if (parts.size() != 1)
      mooseError("Instance-scoped element lookup expects a clean integer id but got '", key, "'.");
    const auto elem_id = MooseUtils::convert<AbaqusID>(key);
    const auto local_index = instance->_part._element_id_to_index.at(elem_id);
    return local_index + instance->_local_to_global_element_index_offset;
  }

  std::vector<std::string> parts;
  MooseUtils::tokenize(key, parts, 1, ".");
  if (parts.size() == 2)
  {
    const auto & instance_name = parts[0];
    const auto elem_id = MooseUtils::convert<AbaqusID>(parts[1]);
    if (!_assembly)
      mooseError("Element reference '", key, "' requires an Assembly but none was parsed.");
    if (!_assembly->_instance.has(instance_name))
      mooseError("Instance '", instance_name, "' not found while resolving element '", key, "'.");
    const auto & inst = _assembly->_instance[instance_name];
    const auto local_index = inst._part._element_id_to_index.at(elem_id);
    return local_index + inst._local_to_global_element_index_offset;
  }
  return _model._element_id_to_index.at(MooseUtils::convert<AbaqusID>(key));
}

const Instance &
AssemblyModel::getInstance(const std::string & name) const
{
  if (_assembly && _assembly->_instance.has(name))
    return _assembly->_instance[name];
  mooseError("Instance '", name, "' not found.");
}

} // namespace Abaqus
