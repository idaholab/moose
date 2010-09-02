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

#ifndef VARIABLESBLOCK_H
#define VARIABLESBLOCK_H

#include "ParserBlock.h"

//libmesh Includes
#include "dof_map.h"

class VariablesBlock;

template<>
InputParameters validParams<VariablesBlock>();

class VariablesBlock: public ParserBlock
{
public:
  VariablesBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();
  virtual void copyNodalValues(const std::string &system_name);
};

#endif //VARIABLESBLOCK_H
