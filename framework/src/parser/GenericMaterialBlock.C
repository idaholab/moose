#include "GenericMaterialBlock.h"
#include "MaterialFactory.h"

GenericMaterialBlock::GenericMaterialBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle)
  :ParserBlock(reg_id, real_id, parent, parser_handle),
   _type(getType())
{
  addParam<std::vector<int> >("block", "The list of blocks for which this material is active on", true);
  addParam<std::vector<std::string> >("coupled_to", "The list of kernels, BCs, materials, or auxillary types which are coupled into this Material", false);
  addParam<std::vector<std::string> >("coupled_as", "The list of names referenced inside of this Material which correspond with the coupled_as objects", false);

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
