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

#ifndef MOOSEREGEX_H
#define MOOSEREGEX_H

#include "trex.h"

#include <string>

class MooseRegEx
{
public:
  MooseRegEx();

  ~MooseRegEx();

  void compile(const std::string & pattern);

  bool search(const std::string & text);

protected:
  void cleanUp();

  TRex *_re;
};

#endif // MOOSEREGEX_H
