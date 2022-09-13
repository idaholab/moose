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
#include <ostream>

class InputParameters;

/**
 * This interface is for classes that want to be called to format InputParameters.  When the
 * syntax tree is traversed, each of these methods are called at the right points to build up
 * a formatted string that can meet a number of different needs.
 */
class SyntaxFormatterInterface
{
public:
  SyntaxFormatterInterface() {}
  virtual ~SyntaxFormatterInterface() = default;

  /**
   * This method is called once at the beginning of the tree traversal and can be used to build up
   * header information
   * @return - The formatted preamble string
   */
  virtual std::string preamble() const { return std::string(); }

  /**
   * This method is called once at the end of the tree traversal and can be used to add any
   * necessary trailing information
   * to the final formatted string.
   * @return - The formatted postscript string
   */
  virtual std::string postscript() const { return std::string(); }

  /**
   * This method is called once at each node in the syntax tree before traversing child nodes.
   * @return - The formatted pre-node traversal string
   */
  virtual std::string preTraverse(short /*depth*/) const { return std::string(); }

  /**
   * This method is called at the beginning of each Node in the tree.  It is typically used to
   * provide formatting necessary
   * when opening new blocks.
   * @return - The formatted block open string
   */
  virtual std::string
  printBlockOpen(const std::string & name, short depth, const std::string & doc) = 0;

  /**
   * This method is called at the end of of each Node in the tree.  It is typically used to provide
   * formatting necessary
   * when closing blocks.
   * @return - The formatted block close string
   */
  virtual std::string printBlockClose(const std::string & name, short depth) const = 0;

  /**
   * This function is called for each InputParameters object stored at a particular node.  It is
   * responsible for formatting the parameters
   * for the current node.
   * @return - The formatted parameters string for a Node.
   */
  virtual std::string printParams(const std::string & prefix,
                                  const std::string & fully_qualified_name,
                                  InputParameters & params,
                                  short depth,
                                  const std::string & search_string,
                                  bool & found) = 0;
};
