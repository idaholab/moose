#include "ExampleAux.h"

template<>
InputParameters validParams<ExampleAux>()
{
  InputParameters params;
  params.set<Real>("value")=0.0;
  return params;
}

ExampleAux::ExampleAux(std::string name,
                         InputParameters parameters,
                         std::string var_name,
                         std::vector<std::string> coupled_to,
                         std::vector<std::string> coupled_as)
  :AuxKernel(name, parameters, var_name, coupled_to, coupled_as),
   _coupled_val(coupledValAux("coupled")),
   _value(_parameters.get<Real>("value"))
{}


Real
ExampleAux::computeValue()
{
  return _coupled_val + _value;
}
