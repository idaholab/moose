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

#include "GenericFunctionsBlock.h"
#include "FunctionFactory.h"
#include "Moose.h"

template<>
InputParameters validParams<GenericFunctionsBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericFunctionsBlock::GenericFunctionsBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params),
  _type(getType())
{
  setClassParams(FunctionFactory::instance()->getValidParams(_type));

  //add variables as prerequisite when we add coupling?
}

void
GenericFunctionsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericFunctionsBlock Object\n";
  std::cerr << "\tFunction: " << getShortName() << "\n";
#endif

  _moose_system.addFunction(_type, getShortName(), getClassParams());
}
