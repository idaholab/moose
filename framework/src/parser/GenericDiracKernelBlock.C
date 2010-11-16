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

#include "GenericDiracKernelBlock.h"
#include "DiracKernelFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericDiracKernelBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericDiracKernelBlock::GenericDiracKernelBlock(const std::string & name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params),
   _type(getType())
{
  setClassParams(DiracKernelFactory::instance()->getValidParams(_type));
}

void
GenericDiracKernelBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericDiracKernelBlock Object\n";
  std::cerr << "DiracKernel:" << _type << ":"
            << "\tname:" << getShortName() << ":";
#endif
  
  _moose_system.addDiracKernel(_type, getShortName(), getClassParams());

  // Add the dirac_kernels to the system
  visitChildren();
}
