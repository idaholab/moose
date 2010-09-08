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

#ifndef PRECONDITIONINGBLOCK_H
#define PRECONDITIONINGBLOCK_H

#include "ParserBlock.h"

class PreconditioningBlock;

template<>
InputParameters validParams<PreconditioningBlock>();

class PreconditioningBlock: public ParserBlock
{
public:
  PreconditioningBlock(const std::string & name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

#endif //PRECONDITIONINGBLOCK_H
