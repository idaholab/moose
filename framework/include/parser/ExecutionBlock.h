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

#if 0   // DEPRECATED - USE EXECUTIONER SYSTEM

#ifndef EXECUTIONBLOCK_H
#define EXECUTIONBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class ExecutionBlock;

template<>
InputParameters validParams<ExecutionBlock>();

class ExecutionBlock: public ParserBlock
{
public:
  ExecutionBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

  

#endif //EXECUTIONBLOCK_H

#endif
