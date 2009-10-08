#include "GenericBCBlock.h"
#include "BCFactory.h"

GenericBCBlock::GenericBCBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, input_file),
   _type(getType())
{
  _block_params.set<std::string>("variable");
  _block_params.set<int>("boundary");

  _class_params = BCFactory::instance()->getValidParams(_type);
}

void
GenericBCBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericBCBlock Object\n";
  std::cerr << "BC: " << _type
            << "\tname: " << getShortName()
            << "\tvariable: " << _block_params.get<std::string>("variable")
            << "\tboundary: " << _block_params.get<int>("boundary")
            << "\tvalue: " << _class_params.get<Real>("value") << std::endl;
#endif

  // TODO: Make the block a vector and loop over it appropriately
  BCFactory::instance()->add(_type, getShortName(), _class_params, 
                             _block_params.get<std::string>("variable"),
                             _block_params.get<int>("boundary"));
  
  
}
