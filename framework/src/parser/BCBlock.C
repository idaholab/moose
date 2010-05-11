#include "BCBlock.h"
#include "InputParameters.h"
#include "Parser.h"
#include "Moose.h"

template<>
InputParameters validParams<BCBlock>()
{
  return validParams<ParserBlock>();
}

BCBlock::BCBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{}

void
BCBlock::execute() 
{
  std::cout << "***BCBlock Class***\n";
  ParserBlock::execute();
}
