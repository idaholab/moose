#include "GenericKernelBlock.h"
#include "KernelFactory.h"

GenericKernelBlock::GenericKernelBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle)
  :ParserBlock(reg_id, real_id, parent, parser_handle),
   _type(getType())
{
  addParam<std::string>("variable", "", "The Kernel Name used in your model", true);
  addParam<int>("block", -1, "The mesh file block for which this kernel is active", true);
  addParam<std::vector<std::string> >("coupled_to", "The list of kernels, BCs, materials, or auxillary types which are coupled into this Kernel", false);
  addParam<std::vector<std::string> >("coupled_as", "The list of names referenced inside of this Kernel which correspond with the coupled_as objects", false);

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

  if (getParamValue<int>("block") < 0)
    KernelFactory::instance()->add(_type, getShortName(), getClassParams(), 
                                   getParamValue<std::string>("variable"),
                                   getParamValue<std::vector<std::string> >("coupled_to"),
                                   getParamValue<std::vector<std::string> >("coupled_as"));
  else
    KernelFactory::instance()->add(_type, getShortName(), getClassParams(), 
                                   getParamValue<std::string>("variable"),
                                   getParamValue<std::vector<std::string> >("coupled_to"),
                                   getParamValue<std::vector<std::string> >("coupled_as"),
                                   getParamValue<int>("block"));
}
