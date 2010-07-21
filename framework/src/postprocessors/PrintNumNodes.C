#include "PrintNumNodes.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<PrintNumNodes>()
{
  InputParameters params = validParams<Postprocessor>();
  return params;
}

PrintNumNodes::PrintNumNodes(std::string name, MooseSystem &moose_system, InputParameters parameters):
  Postprocessor(name, moose_system, parameters)
{
}

Real
PrintNumNodes::getValue()
{
  return _moose_system.getMesh()->n_nodes();
}
