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

#ifndef YAMLFORMATTER_H
#define YAMLFORMATTER_H

#include "SyntaxTree.h"

class YAMLFormatter : public SyntaxTree
{
public:
  YAMLFormatter(std::ostream &out, bool dump_mode);

  virtual void preamble();
  virtual void postscript();

  virtual void printBlockOpen(const std::string &name, short depth, const std::string &type) const;
  virtual void printBlockClose(const std::string &name, short depth) const;
  virtual void printParams(InputParameters &params, short depth) const;



//  virtual void print() const;
//  virtual void postscript();

protected:

  /**
   * Helper method for printing the parts of the YAML Syntax
   */
  void printCloseAndOpen(const std::string & name, const std::string * prev_name) const;

protected:
  std::ostream &_out;
  bool _dump_mode;
  bool _first;
};

#endif /* YAMLFORMATTER_H */
