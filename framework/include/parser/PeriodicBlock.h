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

#ifndef PERIODICBLOCK_H
#define PERIODICBLOCK_H

#include "ParserBlock.h"

class PeriodicBlock;

template<>
InputParameters validParams<PeriodicBlock>();

class PeriodicBlock: public ParserBlock
{
public:
  PeriodicBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

protected:
  bool _executed;
};


  

#endif //PERIODICBLOCK_H
