#include "GenericMaterialBlock.h"
#include "MaterialFactory.h"

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
//          << "\tblock: " << getParamValue<std::vector<int> >("block") << std::endl;
#endif

  std::vector<int> block_vector = getParamValue<std::vector<int> >("block");

  for (std::vector<int>::iterator block=block_vector.begin(); block != block_vector.end(); ++block)
    MaterialFactory::instance()->add(_type, getShortName(), getClassParams(),
                                     *block,
                                     getParamValue<std::vector<std::string> >("coupled_to"),
                                     getParamValue<std::vector<std::string> >("coupled_as"));
}
