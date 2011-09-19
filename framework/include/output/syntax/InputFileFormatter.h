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

#include "SyntaxFormatterInterface.h"

// This class is a big hack job - and probably should be refacted into a tree data structure so that printing
// is more consistent and simple

class InputFileFormatter : public SyntaxFormatterInterface
{
public:
  InputFileFormatter(std::ostream & out, bool dump_mode);

  virtual void print(const std::string & name, const std::string * prev_name, std::vector<InputParameters *> & param_ptrs);

protected:
  /// Helper method for printing the parts of the InputFile Syntax
  void printCloseAndOpen(const std::string & name, const std::string * prev_name) const;

private:
  /// Holds the names of the paramters we've seen for a parsed block so they don't get printed twice
  std::map<std::string, std::set<std::string> > _seen_it;

};

#endif /* INPUTFILEFORMATTER_H */
