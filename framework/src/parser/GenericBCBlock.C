#include "GenericBCBlock.h"
#include "BCFactory.h"
#include "AuxFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericBCBlock>()
{
  InputParameters params = validParams<ParserBlock>();
//  params.addRequiredParam<std::string>("variable", "The BC Name used in your model");
//  params.addRequiredParam<std::vector<int> >("boundary", "The boundary number from your input mesh which corresponds to this boundary");

//  params.addParam<std::vector<std::string> >("coupled_to", "The list of variable names this object is coupled to.");
//  params.addParam<std::vector<std::string> >("coupled_as", "The list of variable names as referenced inside of this object which correspond with the coupled_as names");
  return params;
}

GenericBCBlock::GenericBCBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params),
   _type(getType())
{
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
            << "\tname: " << getShortName();
//            << "\tvariable: " << getParamValue<std::string>("variable") << std::endl;
//          << "\tboundary: " << getParamValue<std::vector<int> >("boundary")
//            << "\tvalue: " << _class_params.get<Real>("value") << std::endl;
#endif
 
  
//  std::vector<int> boundary_vector = getParamValue<std::vector<int> >("boundary");
  
//  for (std::vector<int>::iterator boundary=boundary_vector.begin(); boundary != boundary_vector.end(); ++boundary)
    if (_reg_id == "BCs/*")
      BCFactory::instance()->add(_type, getShortName(), _parser_handle.getMooseSystem(), getClassParams());
    else
      AuxFactory::instance()->addBC(_type, getShortName(), _parser_handle.getMooseSystem(), getClassParams());
}
