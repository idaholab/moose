#include "FunctionsBlock.h"

template<>
InputParameters validParams<FunctionsBlock>()
{
  return validParams<ParserBlock>();
}

FunctionsBlock::FunctionsBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params)
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
