#include "GenericAuxKernelBlock.h"
#include "Factory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericAuxKernelBlock>()
{
  InputParameters params = validParams<ParserBlock>();
//  params.addRequiredParam<std::string>("variable", "The Aux Kernel Name used in your model");
  return params;
}

GenericAuxKernelBlock::GenericAuxKernelBlock(const std::string & name, InputParameters params) :
  ParserBlock(name, params),
  _type(getType())
{
  setClassParams(Factory::instance()->getValidParams(_type));
}

void
GenericAuxKernelBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericAuxKernelBlock Object\n";
  std::cerr << "AuxKernel:" << _type << ":"
            << "\tname:" << getShortName() << std::endl;
#endif

  _parser_handle._problem->addAuxKernel(_type, getShortName(), getClassParams());
}

