#include "GenericKernelBlock.h"
#include "KernelFactory.h"

GenericKernelBlock::GenericKernelBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, input_file),
   _type(getType())
{
  _block_params.set<std::string>("variable");
  _block_params.set<std::string>("type");
  _block_params.set<std::vector<std::string> >("coupled_to");
  _block_params.set<std::vector<std::string> >("coupled_as");

  _class_params = KernelFactory::instance()->getValidParams(_type);
}

void
GenericKernelBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericKernelBlock Object\n";
  std::cerr << "Kernel:" << _type << ":"
            << "\tname:" << getShortName() << ":" 
            << "\tvariable:" << _block_params.get<std::string>("variable") << ":" << std::endl;
#endif

  std::vector<std::string> coupled_to, coupled_as;
  
  KernelFactory::instance()->add(_type, getShortName(), _class_params, 
                                 _block_params.get<std::string>("variable"),
                                 _block_params.get<std::vector<std::string> >("coupled_to"),
                                 _block_params.get<std::vector<std::string> >("coupled_as"));
  
}
