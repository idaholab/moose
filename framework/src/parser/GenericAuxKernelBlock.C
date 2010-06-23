#include "GenericAuxKernelBlock.h"
#include "AuxFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericAuxKernelBlock>()
{
  InputParameters params = validParams<ParserBlock>();
//  params.addRequiredParam<std::string>("variable", "The Aux Kernel Name used in your model");
  return params;
}

GenericAuxKernelBlock::GenericAuxKernelBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system,  params),
   _type(getType())
{
  setClassParams(AuxFactory::instance()->getValidParams(_type));
}

void
GenericAuxKernelBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericAuxKernelBlock Object\n";
  std::cerr << "AuxKernel:" << _type << ":"
            << "\tname:" << getShortName() << std::endl;
#endif

  _moose_system.addAuxKernel(_type, getShortName(), getClassParams());
}
