#include "GenericICBlock.h"
#include "InitialConditionFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericICBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericICBlock::GenericICBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params),
   _type(getType())
{
  setClassParams(InitialConditionFactory::instance()->getValidParams(_type));
}

void
GenericICBlock::execute() 
{
  _moose_system.addInitialCondition(_type, getShortName(), getClassParams(), _parent->getShortName());
}
