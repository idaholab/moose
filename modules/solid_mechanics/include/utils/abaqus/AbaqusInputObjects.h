//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>
#include <map>
#include <memory>
#include <istream>

#include "AbaqusInputParser.h"
#include "Conversion.h"
#include "MooseUtils.h"
#include "MooseError.h"

namespace Abaqus
{
struct Part;
struct Assembly;

/**
 * Store objects of type T accessible under a name and a contiguous id.
 */
template <typename T>
class ObjectStore
{
public:
  ObjectStore() = default;

  /// Get the id for a given name
  std::size_t id(const std::string & name) const;

  ///@{ Look up object by id or name
  T & operator[](std::size_t i);
  const T & operator[](std::size_t i) const;
  T & operator[](const std::string & name) { return _object[id(name)]; }
  const T & operator[](const std::string & name) const { return _object[id(name)]; }
  ///@}

  /// Add a new object to the store. Takes name and constructor parameters, returns id.
  template <typename... Args>
  std::size_t add(const std::string & name, Args &&... args);

  /// check if a key is in the store
  bool has(const std::string & name) const { return _name_to_id.find(name) != _name_to_id.end(); }

  /// number of contained objects
  std::size_t size() const { return _object.size(); }

  ///@{ Iterators
  auto begin() { return _object.begin(); }
  auto begin() const { return _object.begin(); }
  auto end() { return _object.end(); }
  auto end() const { return _object.end(); }
  ///@}

private:
  std::vector<T> _object;
  std::map<std::string, std::size_t> _name_to_id;
};

/**
 * Common base class for objects that can contain sets
 */
struct SetContainer
{
  SetContainer(Part * part) : _part(part) {}

  void processNodeSet(const OptionNode & option);
  void processElementSet(const OptionNode & option);

  const std::vector<std::size_t> & getNodeSet(const std::string & nset) const;
  const std::vector<std::size_t> & getElementSet(const std::string & elset) const;

  /// UEL node sets
  std::map<std::string, std::vector<std::size_t>> _node_set;

  /// UEL element sets
  std::map<std::string, std::vector<std::size_t>> _element_set;

  void processSetHelper(std::map<std::string, std::vector<std::size_t>> & set_map,
                        const OptionNode & option,
                        const std::string & name_key);
  Part * _part;
};

/**
 * Root input file scope
 */
struct Root
{
  Root() = default;
  void process(const BlockNode & root);

  /// Parts
  ObjectStore<Part> _part;

  /// Assemblies
  ObjectStore<Assembly> _assembly;

  /// mesh points and dof bitmask
  std::vector<std::pair<Point, SubdomainID> _mesh_points;

  /// A map from nodes (i.e. node elements) to user elements (ids)
  std::unordered_map<dof_id_type, std::vector<int>> _node_to_uel_map;
};

/**
 * User element option
 */
struct UserElement
{
  UserElement(const OptionNode & option);

  int _coords;
  std::size_t _n_nodes;
  int _n_statev;
  int _n_properties;
  int _n_iproperties;
  bool _symmetric;
  std::string _type;
  std::size_t _type_id;
  std::vector<std::vector<std::size_t>> _vars;

  // bitmask encoding the dof selection in _vars at each node
  std::vector<SubdomainID> _var_mask;
};

/**
 * Part Block
 */
struct Part : public SetContainer
{
  Part(const BlockNode & block);

  ObjectStore<UserElement> _element_definition;

  /// float and int property storage. elements will point to their respective entries in this vector
  std::list<std::pair<std::vector<Real>, std::vector<int>>> _properties;

  struct Element
  {
    std::size_t _type_id;
    std::vector<int> _node_list;
  };

  std::vector<std::pair<int, Point>> _node;
  std::vector<std::pair<int, Element>> _element;
};

/**
 * Instance option
 */
struct Instance : public SetContainer
{
  Instance(std::size_t part_id)
    : SetContainer(*this), _part_id(part_id), _rotation(1, 0, 0, 0, 1, 0, 0, 0, 1)
  {
  }
  Instance(const OptionNode & option, const Root & root);

  // upon instantiation when nodes are created...
  // ...we map local abaqus node ids to global libmesh node/node_element IDs
  std::unordered_map<int, dof_id_type> _libmesh_node_id;
  // ...and abaqus elements to indices into the uel elements in the moose mesh
  std::unordered_map<int, dof_id_type> _moose_elem_id;
};

/**
 * Assembly Block
 */
struct Assembly : public SetContainer
{
  Assembly() = default;
  Assembly(const BlockNode & block, const Root & root);

  ObjectStore<Instance> _instance;
};

/**
 * Step block
 */
struct Step
{
  Step(const BlockNode &, Root &) {}
};

/**
 * Object store functions
 */
template <typename T>
std::size_t
ObjectStore<T>::id(const std::string & name) const
{
  auto it = _name_to_id.find(name);
  if (it != _name_to_id.end())
    return it->second;
  throw std::runtime_error("Invalid name in ObjectStore lookup.");
}

template <typename T>
T &
ObjectStore<T>::operator[](std::size_t i)
{
  if (i < _object.size())
    return _object[i];
  throw std::runtime_error("Invalid index in ObjectStore lookup.");
}

template <typename T>
const T &
ObjectStore<T>::operator[](std::size_t i) const
{
  if (i < _object.size())
    return _object[i];
  throw std::runtime_error("Invalid index in ObjectStore lookup.");
}

template <typename T>
template <typename... Args>
std::size_t
ObjectStore<T>::add(const std::string & name, Args &&... args)
{
  const auto id = _object.size();
  _object.emplace_back(std::forward<Args>(args)...);
  if (_name_to_id.find(name) != _name_to_id.end())
    throw std::runtime_error("Duplicate name in ObjectStore.");
  _name_to_id[name] = id;
}

} // namespace Abaqus
