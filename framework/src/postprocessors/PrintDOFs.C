#include "PrintDOFs.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<PrintDOFs>()
{
  InputParameters params = validParams<Postprocessor>();
  return params;
}

PrintDOFs::PrintDOFs(std::string name, MooseSystem &moose_system, InputParameters parameters):
  Postprocessor(name, moose_system, parameters)
{
}

Real
PrintDOFs::getValue()
{
  return _moose_system.getEquationSystems()->n_dofs();
}
