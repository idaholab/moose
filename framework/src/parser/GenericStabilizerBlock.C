#include "GenericStabilizerBlock.h"
#include "Factory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericStabilizerBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<std::string>("variable", "The name of the variable this Stabilizer will act on.");
  return params;
}

GenericStabilizerBlock::GenericStabilizerBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params),
    _type(getType())
{
  setClassParams(Factory::instance()->getValidParams(_type));
}

void
GenericStabilizerBlock::execute() 
{
//  _moose_system.addStabilizer(_type, getShortName(), getClassParams());
}
