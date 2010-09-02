/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef OUTPUTBLOCK_H
#define OUTPUTBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class OutputBlock;

template<>
InputParameters validParams<OutputBlock>();

class OutputBlock: public ParserBlock
{
public:
  OutputBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

#endif //OUTPUTBLOCK_H
