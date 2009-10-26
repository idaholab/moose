#include "BCsBlock.h"

#include "BCFactory.h"

BCsBlock::BCsBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{}

void
BCsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the BCsBlock Object\n";
#endif

  if (_reg_id == "BCs")
    BoundaryCondition::init();
  
  // Add the BCs or AuxBCs to the system
  visitChildren();
}
