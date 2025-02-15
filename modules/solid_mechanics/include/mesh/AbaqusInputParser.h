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

namespace AbaqusInputParser
{
/**
 * Helper class to get a map of header fields
 */
class HeaderMap
{
public:
  HeaderMap(const std::string & header);
  bool has(const std::string & key) const;
  template <typename T>
  T get(const std::string & key) const;

private:
  std::map<std::string, std::string> _map;
  const std::string _header;
};

class Root;

/**
 * Helper class to build and parse blocks and store them as a list
 */
template <typename T>
class Builder
{
public:
  Builder(const Root & root) : _root(root) {}
  void operator()()
  {
    T block(_root);
    block.parse();
    _list.push_back(block);
  }

private:
  const Root & _root;
  std::vector<T> _list;
};

/**
 * Base class for all blocks
 */
class Block
{
public:
  Block(const Root & root) : _root(root) {}
  Block() = delete;
  Block(const Block &) = delete;
  Block(Block &&) = delete;

  virtual ~Block() = default;

  virtual read() { add<Assembly>(); }

  template <typename T>
  void registerBlock(const std::string & name)
  {
    _blocks[name] = make_unique<Builder<T>>(_root);
  }

private:
  std::map<std::string, std::unique_ptr<Block>> _blocks;
  Root & _root;
};

/**
 * Toplevel class to parse Abaqus input files
 */
class Root : public Block
{
public:
  Root(std::istream file) : Block(*this), _file(file)
  {
    // register subblocks
    registerBlock<Assembly>("assembly");
    registerBlock<Step>("step");

    // load and preprocess entire file
    loadFile();
  }

  Root() = delete;
  Root(const Root &) = delete;
  Root(Root &&) = delete;

private:
  void loadFile();

  /// input steam for the Abaqus input file
  std::istream & _file;

  /// items for each line of the file (without comments, skipping empty lines, and joining continuation lines)
  std::vector<std::vector<std::string>> _lines;

  /// currently parsed line
  std::size_t _current_line;
};

/**
 * Subblock classes
 */
#define ABAQUS_SUBBLOCK(name)                                                                      \
  class name : public Block                                                                        \
  {                                                                                                \
  public:                                                                                          \
    name(const Root & root);                                                                       \
  };

ABAQUS_SUBBLOCK(Part)
ABAQUS_SUBBLOCK(Assembly)
ABAQUS_SUBBLOCK(Step)
ABAQUS_SUBBLOCK(Instance)

} // namespace AbaqusInputParser
