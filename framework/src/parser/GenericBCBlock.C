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

#include "GenericBCBlock.h"
#include "BCFactory.h"
#include "AuxFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericBCBlock>()
{
  InputParameters params = validParams<ParserBlock>();
//  params.addRequiredParam<std::string>("variable", "The BC Name used in your model");
//  params.addRequiredParam<std::vector<int> >("boundary", "The boundary number from your input mesh which corresponds to this boundary");
  return params;
}

GenericBCBlock::GenericBCBlock(const std::string & name, InputParameters params)
  :ParserBlock(name, params),
   _type(getType())
{
  if (Parser::pathContains(name, "BCs"))
    setClassParams(BCFactory::instance()->getValidParams(_type));
  else
    setClassParams(AuxFactory::instance()->getValidParams(_type));
}

void
GenericBCBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericBCBlock Object\n";
  std::cerr << "BC: " << _type
            << "\tname: " << getShortName();
#endif
 
  if (Parser::pathContains(_name, "BCs"))
    _moose_system.addBC(_type, getShortName(), getClassParams());
  else
    _moose_system.addAuxBC(_type, getShortName(), getClassParams());
}
