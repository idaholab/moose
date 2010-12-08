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

#include "GenericAuxKernelBlock.h"
#include "AuxFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericAuxKernelBlock>()
{
  InputParameters params = validParams<ParserBlock>();
//  params.addRequiredParam<std::string>("variable", "The Aux Kernel Name used in your model");
  return params;
}

GenericAuxKernelBlock::GenericAuxKernelBlock(const std::string & name, InputParameters params)
  :ParserBlock(name,  params),
   _type(getType())
{
  setClassParams(AuxFactory::instance()->getValidParams(_type));
}

void
GenericAuxKernelBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericAuxKernelBlock Object\n";
  std::cerr << "AuxKernel:" << _type << ":"
            << "\tname:" << getShortName() << std::endl;
#endif

  _moose_system.addAuxKernel(_type, getShortName(), getClassParams());
}
