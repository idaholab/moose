#include "PeriodicBlock.h"
#include "InputParameters.h"
#include "Parser.h"
#include "Moose.h"

#include "dof_map.h"
#include "mesh_refinement.h"

template<>
InputParameters validParams<PeriodicBlock>()
{
  return validParams<ParserBlock>();
}

PeriodicBlock::PeriodicBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params),
    _executed(false)
{
}

void
PeriodicBlock::execute() 
{
  // If this block has already been executed once... don't do it again.
  if(_executed)
    return;

  _executed = true;

  visitChildren();
}  
