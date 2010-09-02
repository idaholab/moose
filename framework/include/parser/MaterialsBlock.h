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

#ifndef MATERIALSBLOCK_H
#define MATERIALSBLOCK_H

#include "ParserBlock.h"

class MaterialsBlock;

template<>
InputParameters validParams<MaterialsBlock>();

class MaterialsBlock: public ParserBlock
{
public:
  MaterialsBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
};

  

#endif //MATERIALSBLOCK_H
