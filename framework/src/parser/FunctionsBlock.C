#include "FunctionsBlock.h"

template<>
InputParameters validParams<FunctionsBlock>()
{
  return validParams<ParserBlock>();
}

FunctionsBlock::FunctionsBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{
}

void
FunctionsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the FunctionsBlock Object\n";
#endif

  //add the functions to the system
  visitChildren();
}  

  
