/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef JSONINPUTFILEFORMATTER_H
#define JSONINPUTFILEFORMATTER_H

#include "json/json.h"

/**
 * This class produces produces a dump of the InputFileParameters that appears like the normal input
 * file syntax.
 * Different from InputFileFormatter in that it takes its input from JsonSyntaxTree.
 */
class JsonInputFileFormatter
{
public:
  JsonInputFileFormatter();

  /**
   * Returns a string representation of the tree in input file format.
   * @param root The root node of the tree to output.
   */
  std::string toString(const moosecontrib::Json::Value & root);

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
  void
  addBlock(const std::string & name, const moosecontrib::Json::Value & block, bool top = false);

  /**
   * Add a comment to the block. It will add the proper indentation and #.
   * @param comment The comment to add.
   */
  void addParameters(const moosecontrib::Json::Value & params);

  /**
   * Add a dictionary of type blocks to the output.
   * @param key This will be used to get the dictionary of types from block.
   * @param block The Json data that is the parent of the types data.
   */
  void addTypes(const std::string & key, const moosecontrib::Json::Value & block);

  const int _spaces;
  int _level;
  std::ostringstream _stream;
};

#endif /* JSONINPUTFILEFORMATTER_H */
