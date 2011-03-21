#include "GenericICBlock.h"
#include "Factory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericICBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericICBlock::GenericICBlock(const std::string & name, InputParameters params) :
  ParserBlock(name, params),
  _type(getType())
{
  setClassParams(Factory::instance()->getValidParams(_type));
}

void
GenericICBlock::execute() 
{
  _parser_handle._problem->addInitialCondition(_type, getShortName(), getClassParams(), _parent->getShortName());
}

