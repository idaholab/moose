#include "GenericFunctionsBlock.h"
#include "FunctionFactory.h"
#include "Moose.h"

template<>
InputParameters validParams<GenericFunctionsBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  //params.addParam<std::string>("type", "UserFunction", "Specifies the type of function");
  //params.addRequiredParam<std::string>("function", "Symbolic expression");
  //params.addParam<std::vector<std::string> >("constants", std::vector<std::string>(0), "The variables (excluding t,x,y,z) in the symbolic function.");
  //params.addParam<std::vector<Real> >("values", std::vector<Real>(0), "The values that correspond to the variables");

  return params;
}

GenericFunctionsBlock::GenericFunctionsBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params),
  _type(getType())
{
  setClassParams(FunctionFactory::instance()->getValidParams(_type));

  //add variables as prerequisite when we add coupling
}

void
GenericFunctionsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericFunctionsBlock Object\n";
  std::cerr << "\tFunction: " << getShortName() << "\n";
  //std::cerr << "\tEquation: " << getParamValue<std::string>("function") << std::endl;
#endif

  _moose_system.addFunction(_type, getShortName(), getClassParams());

  //TODO not necessary for now, but leave in for future expansion?
  //visitChildren();
}
