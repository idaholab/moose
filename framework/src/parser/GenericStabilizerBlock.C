#include "GenericStabilizerBlock.h"
#include "StabilizerFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericStabilizerBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<std::string>("variable", "The name of the variable this Stabilizer will act on.");
  return params;
}

GenericStabilizerBlock::GenericStabilizerBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params),
   _type(getType())
{
  setClassParams(StabilizerFactory::instance()->getValidParams(_type));
}

void
GenericStabilizerBlock::execute() 
{
  _moose_system.addStabilizer(_type, getShortName(), getClassParams());
}
