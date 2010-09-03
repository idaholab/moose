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

#include "MaterialsBlock.h"

#include "MaterialFactory.h"

template<>
InputParameters validParams<MaterialsBlock>()
{
  return validParams<ParserBlock>();
}

MaterialsBlock::MaterialsBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{
  // Register Materials prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("Preconditioning");
  addPrereq("AuxVariables");
  addPrereq("Executioner");
}

void
MaterialsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the MaterialsBlock Object\n";
#endif

  // Add the Materials to the system
  visitChildren();
}
