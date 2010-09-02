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

#if 0   // DEPRECATED - USE EXECUTIONER SYSTEM

#ifndef TRANSIENTBLOCK_H
#define TRANSIENTBLOCK_H

#include "ParserBlock.h"
#include "InputParameters.h"

//Forward Declarations
class TransientBlock;

template<>
InputParameters validParams<TransientBlock>();

class TransientBlock: public ParserBlock
{
public:
  TransientBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

  void setOutOfOrderTransientParams(Parameters & params) const;
};

  

#endif //TRANSIENTBLOCK_H


#endif
