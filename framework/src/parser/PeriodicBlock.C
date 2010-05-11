#include "PeriodicBlock.h"
#include "InputParameters.h"
#include "Parser.h"
#include "Moose.h"

#include "dof_map.h"

template<>
InputParameters validParams<PeriodicBlock>()
{
  return validParams<ParserBlock>();
}

PeriodicBlock::PeriodicBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params),
   _executed(false)
{}

void
PeriodicBlock::execute() 
{
  // If this block has already been executed once... don't do it again.
  if(_executed)
    return;

  _executed = true;

  visitChildren();
}  
