#include "GenericMaterialBlock.h"
#include "MaterialFactory.h"
#include "Moose.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericMaterialBlock>()
{
  return validParams<ParserBlock>();
}

GenericMaterialBlock::GenericMaterialBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params),
   _type(getType())
{
  setClassParams(MaterialFactory::instance()->getValidParams(_type));
}

void
GenericMaterialBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericMaterialBlock Object\n";
  std::cerr << "Material: " << _type
            << "\tname: " << getShortName() << std::endl;
#endif
  
    _moose_system.addMaterial(_type, getShortName(), getClassParams());
}
