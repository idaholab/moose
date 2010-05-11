#include "PreconditioningBlock.h"
#include "InputParameters.h"
#include "Parser.h"
#include "Moose.h"

template<>
InputParameters validParams<PreconditioningBlock>()
{
  return validParams<ParserBlock>();
}

PreconditioningBlock::PreconditioningBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{}

void
PreconditioningBlock::execute() 
{
  // Execute the preconditioning block
  visitChildren();
}  
