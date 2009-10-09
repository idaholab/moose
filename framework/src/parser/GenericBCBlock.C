#include "GenericBCBlock.h"
#include "BCFactory.h"

GenericBCBlock::GenericBCBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file),
   _type(getType())
{
  _block_params.set<std::string>("variable");
  _block_params.set<std::vector<int> >("boundary");
  _block_params.set<std::vector<std::string> >("coupled_to");
  _block_params.set<std::vector<std::string> >("coupled_as");

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
//          << "\tboundary: " << _block_params.get<std::vector<int> >("boundary")
            << "\tvalue: " << _class_params.get<Real>("value") << std::endl;
#endif

  
  std::vector<int> boundary_vector = _block_params.get<std::vector<int> >("boundary");
  
  for (std::vector<int>::iterator boundary=boundary_vector.begin(); boundary != boundary_vector.end(); ++boundary) 
    BCFactory::instance()->add(_type, getShortName(), _class_params, 
                               _block_params.get<std::string>("variable"),
                               *boundary,
                               _block_params.get<std::vector<std::string> >("coupled_to"),
                               _block_params.get<std::vector<std::string> >("coupled_as"));
}
