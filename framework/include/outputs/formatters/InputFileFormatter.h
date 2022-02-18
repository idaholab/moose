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

/**
 * This class produces produces a dump of the InputParameters that appears like the normal input
 * file syntax.
 */
class InputFileFormatter : public SyntaxTree
{
public:
  InputFileFormatter(bool dump_mode);

  virtual std::string
  printBlockOpen(const std::string & name, short depth, const std::string & /*doc*/) override;
  virtual std::string printBlockClose(const std::string & name, short depth) const override;
  virtual std::string printParams(const std::string & prefix,
                                  const std::string & fully_qualified_name,
                                  InputParameters & params,
                                  short depth,
                                  const std::string & search_string,
                                  bool & found) override;

protected:
  bool _dump_mode;
};
