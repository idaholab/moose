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

#ifndef GENERICAUXKERNELBLOCK_H
#define GENERICAUXKERNELBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericAuxKernelBlock;

template<>
InputParameters validParams<GenericAuxKernelBlock>();

class GenericAuxKernelBlock: public ParserBlock
{
public:
  GenericAuxKernelBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICAUXKERNELBLOCK_H
