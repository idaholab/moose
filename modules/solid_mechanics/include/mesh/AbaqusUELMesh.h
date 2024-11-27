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

  struct UserElement
  {
    std::size_t type_id;
    std::size_t elem_id;
    /// nodes uses 0-based indexing
    std::vector<int> nodes;
  };

  struct AbaqusInputBlock
  {
    AbaqusInputBlock(const std::string & header) : _header(header) {}
    HeaderMap _header;
    std::vector<std::string> _data_lines;
  };

  const std::set<SubdomainID> & getVarBlocks() const { return _uel_block_ids; }
  const std::vector<UELDefinition> & getUELs() const { return _element_definition; }
  const std::vector<UserElement> & getElements() const { return _elements; }
  const std::unordered_map<int, std::vector<int>> & getNodeToUELMap() { return _node_to_uel_map; }
  const UELDefinition & getUEL(const std::string & type);

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
  void readNodeSet(const std::string & header);
  void readElementSet(const std::string & header);
  AbaqusInputBlock readInputBlock(const std::string & header);

  void setupLibmeshSubdomains();
  void setupNodeSets();

  /// Stream object used to interact with the file
  std::unique_ptr<std::istream> _in;

  /// Element type definitions
  std::vector<UELDefinition> _element_definition;

  /// Element type string to type ID lookup
  std::map<std::string, std::size_t> _element_type_to_typeid;

  int _max_node_id;

  /// Element connectivity
  std::vector<UserElement> _elements;

  /// A map from nodes (i.e. node elements) to user elements (ids)
  std::unordered_map<int, std::vector<int>> _node_to_uel_map;

  /// all subdomain IDs used for UEL variable restriction
  std::set<SubdomainID> _uel_block_ids;

  /// UEL node sets
  std::map<std::string, std::vector<std::size_t>> _node_set;

  /// UEL element sets
  std::map<std::string, std::vector<std::size_t>> _element_set;

  /// initial condition data
  std::vector<AbaqusInputBlock> _abaqus_ics;

  /// enable additional debugging output
  const bool _debug;

private:
  void readSetHelper(std::map<std::string, std::vector<std::size_t>> & set,
                     const std::string & header,
                     const std::string & name_key);

  class EndOfAbaqusInput : public std::exception
  {
  };
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
