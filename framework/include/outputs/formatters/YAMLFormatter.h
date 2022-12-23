//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SyntaxTree.h"
#include "InputParameters.h"

#include <sstream>
#include <iterator>

/**
 * This class produces produces a yaml dump of the InputParameters that is machine parsable by
 * any YAML formatter.
 */
class YAMLFormatter : public SyntaxTree
{
public:
  YAMLFormatter(bool dump_mode);

  virtual std::string preamble() const override;
  virtual std::string postscript() const override;

  virtual std::string preTraverse(short depth) const override;
  virtual std::string
  printBlockOpen(const std::string & name, short depth, const std::string & doc) override;
  virtual std::string printBlockClose(const std::string & name, short depth) const override;
  virtual std::string printParams(const std::string & prefix,
                                  const std::string & fully_qualified_name,
                                  InputParameters & params,
                                  short depth,
                                  const std::string & search_string,
                                  bool & found) override;
  template <typename T>
  void addEnumOptionsAndDocs(std::ostringstream & oss, T & param, const std::string & indent);

protected:
  bool _dump_mode;

  /**
   * Method for building an output string that accounts for specific types (e.g., Point)
   * @param output Reference to the output string
   * @param iter InputParameters iterator that is being output
   */
  void buildOutputString(std::ostringstream & output,
                         const std::iterator_traits<InputParameters::iterator>::value_type & p);
};
