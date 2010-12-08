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

#include "StabilizersBlock.h"

#include "StabilizerFactory.h"

template<>
InputParameters validParams<StabilizersBlock>()
{
  return validParams<ParserBlock>();
}

StabilizersBlock::StabilizersBlock(const std::string & name, InputParameters params)
  :ParserBlock(name, params)
{
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("AuxVariables");
  addPrereq("Kernels");
}

void
StabilizersBlock::execute() 
{
  // Add the stabilizers to the system
  visitChildren();
}  
