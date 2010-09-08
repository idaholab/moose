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

#ifndef STABILIZERSBLOCK_H
#define STABILIZERSBLOCK_H

#include "ParserBlock.h"

class StabilizersBlock;

template<>
InputParameters validParams<StabilizersBlock>();

class StabilizersBlock: public ParserBlock
{
public:
  StabilizersBlock(const std::string & name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};


  

#endif //STABILIZERSBLOCK_H
