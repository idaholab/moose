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

#ifndef FUNCTIONSBLOCK_H
#define FUNCTIONSBLOCK_H

#include "ParserBlock.h"

class FunctionsBlock;

template<>
InputParameters validParams<FunctionsBlock>();

/**
 * All this class does is call visitChildren()
 */
class FunctionsBlock : public ParserBlock
{
public:
  FunctionsBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

#endif //FUNCTIONSBLOCK_H
