#include "GenericMaterialBlock.h"
#include "MaterialFactory.h"
#include "Moose.h"

template<>
InputParameters validParams<GenericMaterialBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericMaterialBlock::GenericMaterialBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params),
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

    InputParameters class_params = getClassParams();
    MaterialFactory::instance()->add(_type, getShortName(), *Moose::moose_system, getClassParams());
}
