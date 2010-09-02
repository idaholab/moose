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

#ifndef GENERICPOSTPROCESSORBLOCK_H
#define GENERICPOSTPROCESSORBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericPostprocessorBlock;

template<>
InputParameters validParams<GenericPostprocessorBlock>();

class GenericPostprocessorBlock: public ParserBlock
{
public:
  GenericPostprocessorBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  std::string _type;
};

  

#endif //GENERICPOSTPROCESSORBLOCK_H
