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

#include "PostprocessorsBlock.h"

#include "PostprocessorFactory.h"

template<>
InputParameters validParams<PostprocessorsBlock>()
{
  return validParams<ParserBlock>();
}

PostprocessorsBlock::PostprocessorsBlock(const std::string & name, InputParameters params)
  :ParserBlock(name, params)
{
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("Preconditioning");
  addPrereq("AuxVariables");
  addPrereq("Materials");
}

void
PostprocessorsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the PostprocessorsBlock Object\n";
#endif

  // Add the postprocessors to the system
  visitChildren();
}  

  
