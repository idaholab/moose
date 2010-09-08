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

#include "DampersBlock.h"

#include "DamperFactory.h"

template<>
InputParameters validParams<DampersBlock>()
{
  return validParams<ParserBlock>();
}

DampersBlock::DampersBlock(const std::string & name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("Preconditioning");
  addPrereq("AuxVariables");
  addPrereq("Materials");
}

void
DampersBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the DampersBlock Object\n";
#endif

  // Add the dampers to the system
  visitChildren();
}  

  
