#include "GenericKernelBlock.h"
#include "Factory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericKernelBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<std::string>("variable", "The name of the variable this Kernel will act on.");
  params.addParam<int>("block", "The mesh file block for which this kernel is active.  If not set then this Kernel will be active everywhere.");
  return params;
}


GenericKernelBlock::GenericKernelBlock(const std::string & name, InputParameters params) :
  ParserBlock(name, params),
  _type(getType())
{
  setClassParams(Factory::instance()->getValidParams(_type));
}

void
GenericKernelBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericKernelBlock Object\n";
  std::cerr << "Kernel:" << _type << ":"
            << "\tname:" << getShortName() << ":" << std::endl;
#endif

  _parser_handle._problem->addKernel(_type, getShortName(), getClassParams());
}

