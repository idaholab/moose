#include "GenericAuxKernelBlock.h"
#include "AuxFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericAuxKernelBlock>()
{
  InputParameters params = validParams<ParserBlock>();
//  params.addRequiredParam<std::string>("variable", "The Aux Kernel Name used in your model");
  
//  params.addParam<std::vector<std::string> >("coupled_to", "The list of variable names this Kernel is coupled to.");
//  params.addParam<std::vector<std::string> >("coupled_as", "The list of variable names as referenced inside of this Kernel which correspond with the coupled_as names");
  return params;
}

GenericAuxKernelBlock::GenericAuxKernelBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params),
   _type(getType())
{
  setClassParams(AuxFactory::instance()->getValidParams(_type));
}

void
GenericAuxKernelBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericAuxKernelBlock Object\n";
  std::cerr << "AuxKernel:" << _type << ":"
            << "\tname:" << getShortName() << std::endl;
#endif

  AuxFactory::instance()->add(_type, getShortName(), _parser_handle.getMooseSystem(), getClassParams());  
}
