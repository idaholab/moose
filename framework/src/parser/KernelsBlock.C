#include "KernelsBlock.h"

#include "KernelFactory.h"

KernelsBlock::KernelsBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, input_file)
{
  _block_params.set<std::vector<std::string> >("names");
}

void
KernelsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the KernelsBlock Object\n";
#endif

  // TODO: Setup Time Params

  Kernel::init(Moose::equation_system);
  

  // Add the kernels to the system
  visitChildren();
}
