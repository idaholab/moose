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

#ifndef VARIABLESBLOCK_H
#define VARIABLESBLOCK_H

#include "ParserBlock.h"

//libmesh Includes
#include "dof_map.h"
#include "coupling_matrix.h"

class VariablesBlock;

template<>
InputParameters validParams<VariablesBlock>();

class VariablesBlock: public ParserBlock
{
public:
  VariablesBlock(const std::string & name, MooseSystem & moose_system, InputParameters params);
  virtual ~VariablesBlock();

  virtual void execute();
  virtual void copyNodalValues(const std::string &system_name);

protected:
  CouplingMatrix * _cm;
};

#endif //VARIABLESBLOCK_H
