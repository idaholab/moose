#include "GenericICBlock.h"
#include "InitialConditionFactory.h"

GenericICBlock::GenericICBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle)
  :ParserBlock(reg_id, real_id, parent, parser_handle),
   _type(getType())
{
  addParam<std::string>("variable", "", "The name of the variable to set the initial condition for.", true);

  setClassParams(InitialConditionFactory::instance()->getValidParams(_type));
}

void
GenericICBlock::execute() 
{
  InitialConditionFactory::instance()->add(_type, getShortName(), getClassParams(), _parent->getShortName());
}
