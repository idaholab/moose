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

#include "BCBlock.h"
#include "InputParameters.h"
#include "Parser.h"
#include "Moose.h"

template<>
InputParameters validParams<BCBlock>()
{
  return validParams<ParserBlock>();
}

BCBlock::BCBlock(const std::string & name, InputParameters params)
  :ParserBlock(name, params)
{}

void
BCBlock::execute() 
{
  std::cout << "***BCBlock Class***\n";
  ParserBlock::execute();
}
