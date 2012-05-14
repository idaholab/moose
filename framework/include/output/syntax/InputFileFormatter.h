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

class InputFileFormatter : public SyntaxTree
{
public:
  InputFileFormatter(std::ostream &out, bool dump_mode);

  virtual void printBlockOpen(const std::string &name, short depth, const std::string &type) const;
  virtual void printBlockClose(const std::string &name, short depth) const;
  virtual void printParams(InputParameters &params, short depth) const;

protected:
  std::ostream &_out;
  bool _dump_mode;
};

#endif /* INPUTFILEFORMATTER_H */
