#include "GenericStabilizerBlock.h"
#include "StabilizerFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericStabilizerBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<std::string>("variable", "The name of the variable this Stabilizer will act on.");
  params.addParam<std::vector<std::string> >("coupled_to", "The list of variable names this Stabilizer is coupled to.");
  params.addParam<std::vector<std::string> >("coupled_as", "The list of variable names as referenced inside of this Stabilizer which correspond with the coupled_as names");
  return params;
}

GenericStabilizerBlock::GenericStabilizerBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params),
   _type(getType())
{
  setClassParams(StabilizerFactory::instance()->getValidParams(_type));
}

void
GenericStabilizerBlock::execute() 
{
  StabilizerFactory::instance()->add(_type, getShortName(), _parser_handle.getMooseSystem(), getClassParams());
}
