#include "BCsBlock.h"

#include "BCFactory.h"

BCsBlock::BCsBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, input_file)
{
  _block_params.set<std::vector<std::string> >("names");
}

void
BCsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the KernelsBlock Object\n";
#endif

  BoundaryCondition::init();
  
  // Add the BCs to the system
  visitChildren();
}
