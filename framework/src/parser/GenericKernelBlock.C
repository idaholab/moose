#include "GenericKernelBlock.h"
#include "KernelFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<GenericKernelBlock>()
{
  InputParameters params = validParams<ParserBlock>();
//  params.addRequiredParam<std::string>("variable", "The name of the variable this Kernel will act on.");

//  params.addParam<int>("block", "The mesh file block for which this kernel is active.  If not set then this Kernel will be active everywhere.");
//  params.addParam<std::vector<std::string> >("coupled_to", "The list of variable names this Kernel is coupled to.");
//  params.addParam<std::vector<std::string> >("coupled_as", "The list of variable names as referenced inside of this Kernel which correspond with the coupled_as names");
  return params;
}

GenericKernelBlock::GenericKernelBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params),
   _type(getType())
{
  std::cout<<"Type: "<<_type<<std::endl;
  
  KernelFactory::instance()->getValidParams(_type).print();
  setClassParams(KernelFactory::instance()->getValidParams(_type));
}

void
GenericKernelBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericKernelBlock Object\n";
  std::cerr << "Kernel:" << _type << ":"
            << "\tname:" << getShortName() << ":";
#endif

  if (isParamValid("block"))
    _moose_system.addKernel(_type, getShortName(), getClassParams(), getParamValue<int>("block"));
  else
    _moose_system.addKernel(_type, getShortName(), getClassParams());
}
