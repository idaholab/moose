#include "GenericAuxKernelBlock.h"
#include "AuxFactory.h"

GenericAuxKernelBlock::GenericAuxKernelBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file),
   _type(getType())
{
  _block_params.set<std::string>("variable");
  _block_params.set<std::vector<std::string> >("coupled_to");
  _block_params.set<std::vector<std::string> >("coupled_as");

  _class_params = AuxFactory::instance()->getValidParams(_type);
}

void
GenericAuxKernelBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericAuxKernelBlock Object\n";
  std::cerr << "AuxKernel:" << _type << ":"
            << "\tname:" << getShortName() << ":" 
            << "\tvariable:" << _block_params.get<std::string>("variable") << ":" << std::endl;
#endif

  AuxFactory::instance()->add(_type, getShortName(), _class_params, 
                              _block_params.get<std::string>("variable"),
                              _block_params.get<std::vector<std::string> >("coupled_to"),
                              _block_params.get<std::vector<std::string> >("coupled_as"));
  
}
