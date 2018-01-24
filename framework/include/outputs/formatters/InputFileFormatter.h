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

#ifndef INPUTFILEFORMATTER_H
#define INPUTFILEFORMATTER_H

#include "SyntaxTree.h"

/**
 * This class produces produces a dump of the InputFileParameters that appears like the normal input
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

#endif /* INPUTFILEFORMATTER_H */
