#include "PrintDOFs.h"
#include "SubProblem.h"

template<>
InputParameters validParams<PrintDOFs>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

PrintDOFs::PrintDOFs(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters)
{}

Real
PrintDOFs::getValue()
{
  return _subproblem.es().n_dofs();
}
