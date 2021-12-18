//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include <sstream>
#include "nlohmann/json.h"

/**
 * This class produces produces a dump of the InputParameters that appears like the normal input
 * file syntax.
 * It is different from InputFileFormatter in that it takes its input from JsonSyntaxTree.
 */
class JsonInputFileFormatter
{
public:
  JsonInputFileFormatter();

  /**
   * Returns a string representation of the tree in input file format.
   * @param root The root node of the tree to output.
   */
  std::string toString(const nlohmann::json & root);

protected:
  /**
   * Adds a line to the output. This will put in the proper indentation automatically.
   * @param line The line to add.
   * @param max_line_len Used to determine where to start inline comments.
   * @comment comment Comment to add to the line. It will automatically be broken up over multiple
   * lines if too long.
   */
  void addLine(const std::string & line, size_t max_line_len = 0, const std::string & comment = "");

  /**
   * Adds a new block to the output.
   * @param name Name of the block.
   * @param block Json holding data for the block.
   * @param top Whether this is a top level block.
   */
  void addBlock(const std::string & name, const nlohmann::json & block, bool top = false);

  /**
   * Add a comment to the block. It will add the proper indentation and #.
   * @param comment The comment to add.
   */
  void addParameters(const nlohmann::json & params);

  /**
   * Add a dictionary of type blocks to the output.
   * @param key This will be used to get the dictionary of types from block.
   * @param block The Json data that is the parent of the types data.
   */
  void addTypes(const std::string & key, const nlohmann::json & block);

  const int _spaces;
  int _level;
  std::ostringstream _stream;
};
