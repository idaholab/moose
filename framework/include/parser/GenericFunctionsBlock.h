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

#ifndef GENERICFUNCTIONSBLOCK_H
#define GENERICFUNCTIONSBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericFunctionsBlock;

template<>
InputParameters validParams<GenericFunctionsBlock>();

/**
 * This class parses functions in the [Functions] block and creates them.
 */
class GenericFunctionsBlock: public ParserBlock
{
public:
  GenericFunctionsBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

protected:
  std::string _type;
};

#endif //GENERICFUNCTIONSBLOCK_H
