#include "GenericAuxKernelBlock.h"
#include "AuxFactory.h"

GenericAuxKernelBlock::GenericAuxKernelBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file),
   _type(getType())
{
  addParam<std::string>("variable", "", "The Aux Kernel Name used in your model", true);
  addParam<std::vector<std::string> >("coupled_to", "The list of kernels, BCs, materials, or auxillary types which are coupled into this AuxKernel", false);
  addParam<std::vector<std::string> >("coupled_as", "The list of names referenced inside of this AuxKernel which correspond with the coupled_as objects", false);

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
