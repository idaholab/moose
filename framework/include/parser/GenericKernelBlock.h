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

#ifndef GENERICKERNELBLOCK_H
#define GENERICKERNELBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericKernelBlock;

template<>
InputParameters validParams<GenericKernelBlock>();

class GenericKernelBlock: public ParserBlock
{
public:
  GenericKernelBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICKERNELBLOCK_H
