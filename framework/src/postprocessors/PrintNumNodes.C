#include "PrintNumNodes.h"
#include "SubProblem.h"

template<>
InputParameters validParams<PrintNumNodes>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

PrintNumNodes::PrintNumNodes(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters)
{}

Real
PrintNumNodes::getValue()
{
  return _subproblem.mesh().n_nodes();
}
