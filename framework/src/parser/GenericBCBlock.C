#include "GenericBCBlock.h"
#include "Factory.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<GenericBCBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<std::string>("variable", "The BC Name used in your model");
  params.addRequiredParam<std::vector<int> >("boundary", "The boundary number from your input mesh which corresponds to this boundary");
  return params;
}


GenericBCBlock::GenericBCBlock(const std::string & name, InputParameters params) :
  ParserBlock(name, params),
  _type(getType())
{
  setClassParams(Factory::instance()->getValidParams(_type));
}

void
GenericBCBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericBCBlock Object\n";
  std::cerr << "BC: " << _type
            << "\tname: " << getShortName() << std::endl;
#endif

  if (Parser::pathContains(_name, "BCs"))
    _parser_handle._problem->addBoundaryCondition(_type, getShortName(), getClassParams());
  else
    _parser_handle._problem->addAuxBoundaryCondition(_type, getShortName(), getClassParams());
}

