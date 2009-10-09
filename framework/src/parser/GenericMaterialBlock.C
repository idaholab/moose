#include "GenericMaterialBlock.h"
#include "MaterialFactory.h"

GenericMaterialBlock::GenericMaterialBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file),
   _type(getType())
{
  _block_params.set<std::string>("type");
  _block_params.set<std::vector<int> >("block");
  _block_params.set<std::vector<std::string> >("coupled_to");
  _block_params.set<std::vector<std::string> >("coupled_as");
  
  _class_params = MaterialFactory::instance()->getValidParams(_type);
}

void
GenericMaterialBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericMaterialBlock Object\n";
  std::cerr << "Material: " << _type
            << "\tname: " << getShortName() << std::endl;
//          << "\tblock: " << _block_params.get<std::vector<int> >("block") << std::endl;
#endif

  std::vector<int> block_vector = _block_params.get<std::vector<int> >("block");

  for (std::vector<int>::iterator block=block_vector.begin(); block != block_vector.end(); ++block)
    MaterialFactory::instance()->add(_type, getShortName(), _class_params,
                                     *block,
                                     _block_params.get<std::vector<std::string> >("coupled_to"),
                                     _block_params.get<std::vector<std::string> >("coupled_as"));
}
