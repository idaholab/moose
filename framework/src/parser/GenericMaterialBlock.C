#include "GenericMaterialBlock.h"
#include "MaterialFactory.h"

GenericMaterialBlock::GenericMaterialBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, input_file),
   _type(getType())
{
  _block_params.set<std::string>("type");
  _block_params.set<int>("block");
  
  _class_params = MaterialFactory::instance()->getValidParams(_type);
}

void
GenericMaterialBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericMaterialBlock Object\n";
  std::cerr << "Material: " << _type
            << "\tname: " << getShortName()
            << "\tblock: " << _block_params.get<int>("block") << std::endl;
#endif

  // TODO: Make the block param a vector and loop over it appropriately  
  MaterialFactory::instance()->add(_type, getShortName(), _class_params, 
                                   _block_params.get<int>("block"));
  
}
