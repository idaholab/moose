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

#include "DiracKernelsBlock.h"

#include "DiracKernelFactory.h"

template<>
InputParameters validParams<DiracKernelsBlock>()
{
  return validParams<ParserBlock>();
}

DiracKernelsBlock::DiracKernelsBlock(const std::string & name, MooseSystem & moose_system, InputParameters params)
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
DiracKernelsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the DiracKernelsBlock Object\n";
#endif

  // Add the dirac_kernels to the system
  visitChildren();
}  

  
