#include "GenericDiracKernelBlock.h"
#include "Factory.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<GenericDiracKernelBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  return params;
}

GenericDiracKernelBlock::GenericDiracKernelBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params),
    _type(getType())
{
  setClassParams(Factory::instance()->getValidParams(_type));
}

void
GenericDiracKernelBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericDiracKernelBlock Object\n";
  std::cerr << "DiracKernel:" << _type << ":"
            << "\tname:" << getShortName() << ":";
#endif
  
//  _parser_handle._problem->addDiracKernel(_type, getShortName(), getClassParams());

  // Add the dirac_kernels to the system
  visitChildren();
}
