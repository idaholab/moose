#include "GenericICBlock.h"
#include "InitialConditionFactory.h"

template<>
InputParameters validParams<GenericICBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericICBlock::GenericICBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params),
   _type(getType())
{
  setClassParams(InitialConditionFactory::instance()->getValidParams(_type));
}

void
GenericICBlock::execute() 
{
  InitialConditionFactory::instance()->add(_type, getShortName(), *Moose::moose_system, getClassParams(), _parent->getShortName());
}
