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

#ifndef DIRACKERNELSBLOCK_H
#define DIRACKERNELSBLOCK_H

#include "ParserBlock.h"

class DiracKernelsBlock;

template<>
InputParameters validParams<DiracKernelsBlock>();

class DiracKernelsBlock: public ParserBlock
{
public:
  DiracKernelsBlock(const std::string & name, InputParameters params);

  virtual void execute();
};


  

#endif //DIRACKERNELSBLOCK_H
