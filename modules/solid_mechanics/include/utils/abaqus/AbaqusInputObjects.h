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
struct AssemblyRoot;

// Any Abaqus ID should use this type
using AbaqusID = int;

// Any index into a std::vector should use this type
using Index = std::size_t;

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

  /// merge object stores
  void merge(const ObjectStore & other);

private:
  std::vector<T> _object;
  std::map<std::string, std::size_t> _name_to_id;
};

/**
 * User element option
 */
struct UserElement
{
  UserElement(const OptionNode & option);

  /// needed by the UEL plugin
  int _coords;
  std::size_t _n_nodes;
  int _n_statev;
  int _n_properties;
  int _n_iproperties;
  bool _symmetric;

  std::string _type;    // TODO: is this needed?
  std::size_t _type_id; // TODO: is this needed?

  /// list of abaqus variables active at each node
  std::vector<std::vector<std::size_t>> _vars;

  /// bitmask encoding the DOF selection in _vars at each node
  std::vector<SubdomainID> _var_mask;
};

/**
 * Mesh node
 */
struct Node
{
  /// original Abaqus ID (unique only at the part level)
  AbaqusID _id;

  /// location of the node
  Point _point;

  /// bit set for every Abaqus Variable number (-1) that is active on this node
  SubdomainID _var_mask;
};

/**
 * Mesh element
 */
struct Element
{
  /// original Abaqus ID (unique only at the part level)
  AbaqusID _id;

  /// UEL Defintion
  const UserElement & _uel;

  /// index into the corresponding node list (either at part level or root level)
  std::vector<Index> _nodes;

  /// pointer to the assigned property list
  std::pair<Real *, int *> _properties;
};

/**
 * Part Block
 */
struct Part
{
  Part() = default;
  Part(const BlockNode & block) { parse(block); }
  void parse(const BlockNode & block);

  void optionFunc(const std::string & key, const OptionNode & option);

  void processNodeSet(const OptionNode & option);
  void processElementSet(const OptionNode & option);

  const std::vector<std::size_t> & getNodeSet(const std::string & nset) const;
  const std::vector<std::size_t> & getElementSet(const std::string & elset) const;

  void processSetHelper(std::map<std::string, std::vector<Index>> & set_map,
                        std::unordered_map<AbaqusID, Index> & id_to_index,
                        const OptionNode & option,
                        const std::string & name_key);

  ObjectStore<UserElement> _element_definition;

  /// float and int property storage. elements will point to their respective entries in this vector
  std::list<std::pair<std::vector<Real>, std::vector<int>>> _properties;

  /// connectivity
  std::vector<Node> _nodes;
  std::vector<Element> _elements;

  /// map from abaqus node id to part local index
  std::unordered_map<AbaqusID, Index> _node_id_to_index;
  std::unordered_map<AbaqusID, Index> _element_id_to_index;

  /// sets
  std::map<std::string, std::vector<Index>> _elsets;
  std::map<std::string, std::vector<Index>> _nsets;
};

/**
 * Instance option
 */
struct Instance
{
  Instance(const BlockNode & option, AssemblyRoot & root);

  // upon instantiation when nodes are created we map
  // part local indices to global indices in the Root
  Index _local_to_global_node_index_offset;
  Index _local_to_global_element_index_offset;
};

/**
 * Assembly Block
 */
struct Assembly
{
  Assembly() = default;
  Assembly(const BlockNode & block, AssemblyRoot & root) { parse(block, root); }
  void parse(const BlockNode & block, AssemblyRoot & root);

  ObjectStore<Instance> _instance;
};

/**
 * Root input file scope
 */
struct Root : public Part
{
  Root() = default;
  virtual void parse(const BlockNode & root) = 0;

  /// mesh points and dof bitmask
  std::vector<std::pair<Point, SubdomainID>> _mesh_points;

  /// A map from nodes (i.e. node elements) to user elements (ids)
  std::unordered_map<dof_id_type, std::vector<int>> _node_to_uel_map;
};

/**
 * Root node for flat input files
 */
struct FlatRoot : public Root
{
  FlatRoot() = default;
  virtual void parse(const BlockNode & root);
};

/**
 * Root node for assembly based input files
 */
struct AssemblyRoot : public Root
{
  AssemblyRoot() = default;
  virtual void parse(const BlockNode & root);

  /// Parts
  ObjectStore<Part> _part;

  /// Assemblies
  std::unique_ptr<Assembly> _assembly;
};

/**
 * Step block
 */
struct Step
{
  Step(const BlockNode &, Root &) {}
};

/**
 * Essential boundary condition (Dirichlet)
 */
struct Boundary
{
  Boundary(const OptionNode & option, Root & root);

  /// list of nodes this BC applies to
  std::vector<dof_id_type> _node_set;
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
  return id;
}

template <typename T>
void
ObjectStore<T>::merge(const ObjectStore & other)
{
  const auto offset = _object.size();
  _object.insert(_object.end(), other._object.begin(), other._object.end());
  for (const auto & [name, id] : other._name_to_id)
    _name_to_id[name] = id + offset;
}

} // namespace Abaqus
