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

namespace Abaqus
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
  template <typename T>
  T get(const std::string & key, T default_value) const;

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
  BlockNode(std::vector<std::string> line) : Node(line) {}
  std::string stringify(const std::string & indent = "") const;

  template <typename T>
  void forBlock(const std::string & name, T && func) const
  {
    for (const auto & child : _children)
      if (child._type == name)
        func(child);
  }
  template <typename T>
  void forOption(const std::string & name, T && func) const
  {
    for (const auto & option : _options)
      if (option._type == name)
        func(option);
  }

  std::vector<BlockNode> _children;
  std::vector<OptionNode> _options;
};

/**
 * Build a syntax tree from an abaqus input file
 */
class InputParser : public BlockNode
{
public:
  InputParser() : BlockNode({"ROOT"}) {}
  void parse(std::istream & in);

private:
  void loadFile(std::istream & in);

  BlockNode parseBlock(const std::string & end,
                       const std::vector<std::string> & head_line = {"ROOT"});
  OptionNode parseOption(const std::vector<std::string> & head_line = {"ROOT"});

  void parseBlockInternal(BlockNode & node, const std::string & end = "");

  /// items for each line of the file (without comments, skipping empty lines, and joining continuation lines)
  std::vector<std::vector<std::string>> _lines;

  /// currently parsed line
  std::size_t _current_line;
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

template <typename T>
T
HeaderMap::get(const std::string & key, T default_value) const
{
  return has(key) ? get<T>(key) : default_value;
}

template <>
bool HeaderMap::get(const std::string & key) const;

} // namespace Abaqus
