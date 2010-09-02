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

#ifndef MESHGENERATIONBLOCK_H
#define MESHGENERATIONBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class MeshGenerationBlock;

template<>
InputParameters validParams<MeshGenerationBlock>();

class MeshGenerationBlock: public ParserBlock
{
public:
  MeshGenerationBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
  
private:
  bool _executed;
};

#endif //MESHGENERATIONBLOCK_H
