#include "GenericFunctionsBlock.h"
#include "Factory.h"
#include "Moose.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<GenericFunctionsBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericFunctionsBlock::GenericFunctionsBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params),
    _type(getType())
{
  setClassParams(Factory::instance()->getValidParams(_type));
}

void
GenericFunctionsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericFunctionsBlock Object\n";
  std::cerr << "\tFunction: " << getShortName() << "\n";
#endif

  _parser_handle._problem->addFunction(_type, getShortName(), getClassParams());
}
