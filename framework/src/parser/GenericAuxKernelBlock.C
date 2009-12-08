#include "GenericAuxKernelBlock.h"
#include "AuxFactory.h"

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
            << "\tname:" << getShortName() << ":" 
            << "\tvariable:" << getParamValue<std::string>("variable") << ":" << std::endl;
#endif

  AuxFactory::instance()->add(_type, getShortName(), getClassParams(), 
                              getParamValue<std::string>("variable"),
                              getParamValue<std::vector<std::string> >("coupled_to"),
                              getParamValue<std::vector<std::string> >("coupled_as"));
  
}
