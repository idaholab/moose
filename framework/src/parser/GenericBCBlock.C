#include "GenericBCBlock.h"
#include "BCFactory.h"
#include "AuxFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericBCBlock>()
{
  InputParameters params = validParams<ParserBlock>();
//  params.addRequiredParam<std::string>("variable", "The BC Name used in your model");
//  params.addRequiredParam<std::vector<int> >("boundary", "The boundary number from your input mesh which corresponds to this boundary");
  return params;
}

GenericBCBlock::GenericBCBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params),
   _type(getType())
{
  if (Parser::pathContains(name, "BCs"))
    setClassParams(BCFactory::instance()->getValidParams(_type));
  else
    setClassParams(AuxFactory::instance()->getValidParams(_type));
}

void
GenericBCBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericBCBlock Object\n";
  std::cerr << "BC: " << _type
            << "\tname: " << getShortName();
#endif
 
  if (Parser::pathContains(_name, "BCs"))
    _moose_system.addBC(_type, getShortName(), getClassParams());
  else
    _moose_system.addAuxBC(_type, getShortName(), getClassParams());
}
