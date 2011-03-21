#include "GenericMeshModifierBlock.h"
#include "Factory.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<GenericMeshModifierBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericMeshModifierBlock::GenericMeshModifierBlock(const std::string & name, InputParameters params) :
  ParserBlock(name, params),
  _type(getType())
{
  setClassParams(Factory::instance()->getValidParams(_type));
}

void
GenericMeshModifierBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericMeshModifierBlock Object\n";
  std::cerr << "MeshModifier:" << _type << ":"
            << "\tname:" << getShortName() << std::endl;
#endif

  _parser_handle._mesh->addMeshModifer(_type, getShortName(), getClassParams());
}

