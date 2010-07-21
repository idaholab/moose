#include "PrintNumElems.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<PrintNumElems>()
{
  InputParameters params = validParams<Postprocessor>();
  return params;
}

PrintNumElems::PrintNumElems(std::string name, MooseSystem &moose_system, InputParameters parameters):
  Postprocessor(name, moose_system, parameters)
{
}

Real
PrintNumElems::getValue()
{
  return _moose_system.getMesh()->n_elem();
}
