#include "GenericKernelBlock.h"
#include "KernelFactory.h"

GenericKernelBlock::GenericKernelBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params),
   _type(getType())
{
  setClassParams(KernelFactory::instance()->getValidParams(_type));
}

void
GenericKernelBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericKernelBlock Object\n";
  std::cerr << "Kernel:" << _type << ":"
            << "\tname:" << getShortName() << ":" 
            << "\tvariable:" << getParamValue<std::string>("variable") << ":" << std::endl;
#endif

  if (isParamValid("block"))
    KernelFactory::instance()->add(_type, getShortName(), getClassParams(), 
                                   getParamValue<std::string>("variable"),
                                   getParamValue<std::vector<std::string> >("coupled_to"),
                                   getParamValue<std::vector<std::string> >("coupled_as"),
                                   getParamValue<int>("block"));
  else
    KernelFactory::instance()->add(_type, getShortName(), getClassParams(), 
                                   getParamValue<std::string>("variable"),
                                   getParamValue<std::vector<std::string> >("coupled_to"),
                                   getParamValue<std::vector<std::string> >("coupled_as"));
  
}
