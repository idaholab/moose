#include "GenericBCBlock.h"
#include "BCFactory.h"
#include "AuxFactory.h"

GenericBCBlock::GenericBCBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle)
  :ParserBlock(reg_id, real_id, parent, parser_handle),
   _type(getType())
{
  addParam<std::string>("variable", "", "The BC Name used in your model", true);
  addParam<std::vector<int> >("boundary", "The boundary number from your input mesh which corresponds to this boundary", true);
  addParam<std::vector<std::string> >("coupled_to", "The list of kernels, BCs, materials, or auxillary types which are coupled into this AuxKernel", false);
  addParam<std::vector<std::string> >("coupled_as", "The list of names referenced inside of this AuxKernel which correspond with the coupled_as objects", false);
  
  if (reg_id == "BCs/*")
    setClassParams(BCFactory::instance()->getValidParams(_type));
  else
    setClassParams(AuxFactory::instance()->getValidParams(_type));
}

void
GenericBCBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericBCBlock Object\n";
  std::cerr << "BC: " << _type
            << "\tname: " << getShortName()
            << "\tvariable: " << getParamValue<std::string>("variable") << std::endl;
//          << "\tboundary: " << getParamValue<std::vector<int> >("boundary")
//            << "\tvalue: " << _class_params.get<Real>("value") << std::endl;
#endif

  
  std::vector<int> boundary_vector = getParamValue<std::vector<int> >("boundary");

  for (std::vector<int>::iterator boundary=boundary_vector.begin(); boundary != boundary_vector.end(); ++boundary)
    if (_reg_id == "BCs/*")
      BCFactory::instance()->add(_type, getShortName(), getClassParams(), 
                                 getParamValue<std::string>("variable"),
                                 *boundary,
                                 getParamValue<std::vector<std::string> >("coupled_to"),
                                 getParamValue<std::vector<std::string> >("coupled_as"));
    else
      AuxFactory::instance()->addBC(_type, getShortName(), getClassParams(), 
                                  getParamValue<std::string>("variable"),
                                  *boundary,
                                  getParamValue<std::vector<std::string> >("coupled_to"),
                                  getParamValue<std::vector<std::string> >("coupled_as"));
}
