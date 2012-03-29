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

#include "SyntaxFormatterInterface.h"

class YAMLFormatter : public SyntaxFormatterInterface
{
public:
  YAMLFormatter(std::ostream & out, bool dump_mode);

  virtual void preamble();
  virtual void print(const std::string & name, const std::string * /*prev_name*/, std::vector<InputParameters *> & param_ptrs);
  virtual void postscript();

protected:

  /**
   * Helper method for printing the parts of the YAML Syntax
   */
  void printCloseAndOpen(const std::string & name, const std::string * prev_name) const;

private:
  bool first;
};

#endif /* YAMLFORMATTER_H */
