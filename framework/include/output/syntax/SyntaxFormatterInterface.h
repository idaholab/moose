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

#ifndef SYNTAXFORMATTERINTERFACE_H
#define SYNTAXFORMATTERINTERFACE_H

#include "InputParameters.h"

#include <string>
#include <ostream>

class SyntaxFormatterInterface
{
public:
  SyntaxFormatterInterface(std::ostream & out, bool dump_mode=false);
  virtual ~SyntaxFormatterInterface();

  virtual void preamble() {}
  virtual void print(const std::string & name, const std::string * prev_name, std::vector<InputParameters *> & param_ptrs) = 0;
  virtual void postscript() {}
  
protected:
  std::ostream & _out;
  bool _dump_mode;               ///< Indicates whether this is a full dump or input file based
};

#endif /* SYNTAXFORMATTERINTERFACE_H */
