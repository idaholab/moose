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

struct InputNode
{
  InputNode(std::vector<std::string> line);
  virtual ~InputNode() = default;
  virtual std::string stringify(const std::string & indent = "") const = 0;

  std::string _type;
  HeaderMap _header;

  // some blocks have data lines (like Instance for example)
  std::vector<std::vector<std::string>> _data;
};

struct OptionNode : public InputNode
{
  OptionNode(std::vector<std::string> line) : InputNode(line) {}
  virtual std::string stringify(const std::string & indent = "") const;
};

struct BlockNode;
struct BlockNode : public InputNode
{
  BlockNode(std::vector<std::string> line) : InputNode(line) {}
  virtual std::string stringify(const std::string & indent = "") const;

  template <typename To, typename Tc>
  void forAll(To && option_func, Tc && block_func) const
  {
    for (const auto & child : _children)
    {
      const auto & key = child->_type;
      auto * option = dynamic_cast<OptionNode *>(child.get());
      auto * block = dynamic_cast<BlockNode *>(child.get());

      if (option)
      {
        if constexpr (!std::is_same_v<To, std::nullptr_t>)
          option_func(key, *option);
      }
      else if (block)
      {
        if constexpr (!std::is_same_v<Tc, std::nullptr_t>)
          block_func(key, *block);
      }
      else
        throw std::runtime_error("Internal error (mismatching block type)");
    }
  }

  std::vector<std::unique_ptr<InputNode>> _children;
};

/**
 * Build a syntax tree from an abaqus input file
 */
class InputParser : public BlockNode
{
public:
  InputParser() : BlockNode({"ROOT"}) {}
  void parse(std::istream & in);

  bool isFlat() const { return _is_flat; }

private:
  void loadFile(std::istream & in);

  std::unique_ptr<BlockNode> parseBlock(const std::string & end,
                                        const std::vector<std::string> & head_line = {"ROOT"});
  std::unique_ptr<OptionNode> parseOption(const std::vector<std::string> & head_line = {"ROOT"});

  void parseBlockInternal(BlockNode & node, const std::string & end = "");

  /// items for each line of the file (without comments, skipping empty lines, and joining continuation lines)
  std::vector<std::vector<std::string>> _lines;

  /// currently parsed line
  std::size_t _current_line;

  /// are we parsing a flat or assembly based input?
  bool _is_flat;
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
