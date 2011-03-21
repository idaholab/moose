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

#include "PreconditioningBlock.h"
#include "InputParameters.h"
#include "Parser.h"
#include "Moose.h"

template<>
InputParameters validParams<PreconditioningBlock>()
{
  return validParams<ParserBlock>();
}

PreconditioningBlock::PreconditioningBlock(const std::string & name, InputParameters params)
  :ParserBlock(name, params)
{
  // Register the Preconditioning Prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
}

void
PreconditioningBlock::execute() 
{
  // Execute the preconditioning block
  visitChildren();
}  
