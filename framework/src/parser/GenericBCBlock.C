#include "GenericBCBlock.h"
#include "BCFactory.h"
#include "AuxFactory.h"

GenericBCBlock::GenericBCBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file),
   _type(getType())
{
  _block_params.set<std::string>("variable");
  _block_params.set<std::vector<int> >("boundary");
  _block_params.set<std::vector<std::string> >("coupled_to");
  _block_params.set<std::vector<std::string> >("coupled_as");
  
  if (reg_id == "BCs/*")
    _class_params = BCFactory::instance()->getValidParams(_type);
  else
    _class_params = AuxFactory::instance()->getValidParams(_type);
}

void
GenericBCBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericBCBlock Object\n";
  std::cerr << "BC: " << _type
            << "\tname: " << getShortName()
            << "\tvariable: " << _block_params.get<std::string>("variable") << std::endl;
//          << "\tboundary: " << _block_params.get<std::vector<int> >("boundary")
//            << "\tvalue: " << _class_params.get<Real>("value") << std::endl;
#endif

  
  std::vector<int> boundary_vector = _block_params.get<std::vector<int> >("boundary");

  for (std::vector<int>::iterator boundary=boundary_vector.begin(); boundary != boundary_vector.end(); ++boundary)
    if (_reg_id == "BCs/*")
      BCFactory::instance()->add(_type, getShortName(), _class_params, 
                                 _block_params.get<std::string>("variable"),
                                 *boundary,
                                 _block_params.get<std::vector<std::string> >("coupled_to"),
                                 _block_params.get<std::vector<std::string> >("coupled_as"));
    else
      AuxFactory::instance()->addBC(_type, getShortName(), _class_params, 
                                  _block_params.get<std::string>("variable"),
                                  *boundary,
                                  _block_params.get<std::vector<std::string> >("coupled_to"),
                                  _block_params.get<std::vector<std::string> >("coupled_as"));
}
