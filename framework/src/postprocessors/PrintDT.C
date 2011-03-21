#include "PrintDT.h"
#include "SubProblem.h"

template<>
InputParameters validParams<PrintDT>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

PrintDT::PrintDT(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters)
{}

Real
PrintDT::getValue()
{
  return _problem.dt();
}
