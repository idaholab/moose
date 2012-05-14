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
  SyntaxFormatterInterface() {}
  virtual ~SyntaxFormatterInterface() {}
  virtual void preamble() {}
  virtual void postscript() {}

  virtual void print() const = 0;
  virtual void preTraverse(short depth) const {}
  virtual void printBlockOpen(const std::string &name, short depth, const std::string &type) const = 0;
  virtual void printBlockClose(const std::string &name, short depth) const = 0;
  virtual void printParams(InputParameters &params, short depth) const = 0;
};

#endif /* SYNTAXFORMATTERINTERFACE_H */
