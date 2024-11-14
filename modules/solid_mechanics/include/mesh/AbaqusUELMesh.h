// AbaqusUELMesh//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"

#include <stdexcept>

/**
 * Coupling user object to use Abaqus UEXTERNALDB subroutines in MOOSE
 */
class AbaqusUELMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  AbaqusUELMesh(const InputParameters & parameters);
  AbaqusUELMesh(const AbaqusUELMesh & other_mesh);

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

protected:
  /// read a single line from the input
  std::string readLine();

  /// read data line, including continuation lines
  bool readDataLine(std::string & s);

  bool startsWith(const std::string & s, const std::string & pattern);
  void strip(std::string & line);

  void readNodes();
  void readUserElement(const std::string & header);
  void readElements(const std::string & header);

  void setupNodeSets();

  /// Stream object used to interact with the file
  std::unique_ptr<std::istream> _in;

  /// Element type definitions
  struct UELDefinition
  {
    int coords;
    std::size_t nodes;
    int nvars;
    int properties;
    bool symmetric;
    std::string type;
    std::vector<std::vector<std::size_t>> vars;
  };
  std::vector<UELDefinition> _element_definition;

  /// Element type string to type ID lookup
  std::map<std::string, std::size_t> _element_type_to_typeid;

  int _max_node_id;

  /// Element connectivity, this uses 1-based indexing (_elements[0] is unused)
  struct UserElement
  {
    std::size_t type_id;
    std::size_t elem_id;
    /// nodes uses 0-based indexing
    std::vector<int> nodes;
  };
  std::vector<UserElement> _elements;

  /// A map from nodes (i.e. node elements) to user elements (ids)
  std::unordered_map<int, std::vector<int>> _node_to_uel_map;

  ///@{ nodesets for boundary restricting variables
  std::vector<BoundaryName> _nodeset_names;
  std::vector<BoundaryID> _nodeset_ids;
  ///@}

  /// A map from variable numbers to nodeset index
  std::unordered_map<std::size_t, std::size_t> _var_to_nodeset;

private:
  class EndOfAbaqusInput : public std::exception
  {
  };
};

/**
 * Helper class to get a map of header fields
 */
class HeaderMap
{
public:
  HeaderMap(const std::string & header);
  bool has(const std::string & key);
  template <typename T>
  T get(const std::string & key);

private:
  std::map<std::string, std::string> _map;
  const std::string _header;
};

template <typename T>
T
HeaderMap::get(const std::string & key)
{
  const auto it = _map.find(MooseUtils::toUpper(key));
  if (it == _map.end())
    mooseError("Key '", key, "' not found in header\n", _header);
  return MooseUtils::convert<T>(it->second);
}

template <>
bool HeaderMap::get(const std::string & key);
