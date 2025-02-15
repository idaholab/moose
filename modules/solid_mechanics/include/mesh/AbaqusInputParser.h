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

#include "Conversion.h"
#include "MooseUtils.h"
#include "MooseError.h"

namespace AbaqusInputParser
{

/**
 * Helper class to get a map of header fields from a header string
 */
class HeaderMap
{
public:
  HeaderMap() {}
  HeaderMap(const std::vector<std::string> fields);
  std::string stringify(const std::string & indent = "") const;

  bool has(const std::string & key) const;
  template <typename T>
  T get(const std::string & key) const;

private:
  std::map<std::string, std::string> _map;
};

struct Node
{
  Node(std::vector<std::string> line);

  std::string _type;
  HeaderMap _header;
};
struct OptionNode : public Node
{
  OptionNode(std::vector<std::string> line) : Node(line) {}
  std::string stringify(const std::string & indent = "") const;

  std::vector<std::vector<std::string>> _data;
};

struct BlockNode;
struct BlockNode : public Node
{
  BlockNode() : Node({"ROOT"}) {}
  BlockNode(std::vector<std::string> line) : Node(line) {}
  std::string stringify(const std::string & indent = "") const;

  std::vector<BlockNode> _children;
  std::vector<OptionNode> _options;
};

/**
 * Build a syntax tree from an abaqus input file
 */
class AbaqusInputParser
{
public:
  AbaqusInputParser(std::istream & file);
  std::string stringify() const { return _root.stringify(); }

private:
  void loadFile();

  BlockNode parseBlock(const std::string & end = "",
                       const std::vector<std::string> & head_line = {"ROOT"});
  OptionNode parseOption(const std::vector<std::string> & head_line = {"ROOT"});

  /// input steam for the Abaqus input file
  std::istream & _file;

  /// items for each line of the file (without comments, skipping empty lines, and joining continuation lines)
  std::vector<std::vector<std::string>> _lines;

  /// currently parsed line
  std::size_t _current_line;

  BlockNode _root;
};

template <typename T>
T
HeaderMap::get(const std::string & key) const
{
  const auto it = _map.find(MooseUtils::toLower(key));
  if (it == _map.end())
    mooseError("Key '", key, "' not found.");
  return MooseUtils::convert<T>(it->second);
}

template <>
bool HeaderMap::get(const std::string & key) const;

} // namespace AbaqusInputParser
