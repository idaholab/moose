#include "GenericStabilizerBlock.h"
#include "Factory.h"
#include "Parser.h"
#include "MProblem.h"

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
  if (_parser_handle._problem != NULL)
    _parser_handle._problem->addStabilizer(_type, getShortName(), getClassParams());
}
