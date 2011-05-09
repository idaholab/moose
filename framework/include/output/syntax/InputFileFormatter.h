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

class InputFileFormatter : public SyntaxFormatterInterface
{
public:
  InputFileFormatter(std::ostream & out, bool dump_mode);

  virtual void print(const std::string & name, const std::string * prev_name, std::vector<InputParameters *> & param_ptrs) const;

protected:
  /// Helper method for printing the parts of the InputFile Syntax
  void printCloseAndOpen(const std::string & name, const std::string * prev_name) const;
};

#endif /* INPUTFILEFORMATTER_H */
