#include "GenericMaterialBlock.h"
#include "Factory.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<GenericMaterialBlock>()
{
  return validParams<ParserBlock>();
}


GenericMaterialBlock::GenericMaterialBlock(const std::string & name, InputParameters params) :
  ParserBlock(name, params),
  _type(getType())
{
  setClassParams(Factory::instance()->getValidParams(_type));
}

void
GenericMaterialBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericMaterialBlock Object\n";
  std::cerr << "Material: " << _type
            << "\tname: " << getShortName() << std::endl;
#endif

  _parser_handle._problem->addMaterial(_type, getShortName(), getClassParams());
}

