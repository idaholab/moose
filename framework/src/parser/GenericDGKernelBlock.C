#include "GenericDGKernelBlock.h"
#include "DGKernelFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericDGKernelBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericDGKernelBlock::GenericDGKernelBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params),
   _type(getType())
{
  setClassParams(DGKernelFactory::instance()->getValidParams(_type));
}

void
GenericDGKernelBlock::execute()
{
#ifdef DEBUG
  std::cerr << "Inside the GenericDGKernelBlock Object\n";
  std::cerr << "DGKernel: " << _type
            << "\tname: " << getShortName();
#endif
 
  _moose_system.addDGKernel(_type, getShortName(), getClassParams());
}
