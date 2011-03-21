#include "Coupleable.h"
#include "Moose.h"
#include "SubProblem.h"

namespace Moose {

Coupleable::Coupleable(InputParameters & parameters)
{
  SubProblem & subproblem = *parameters.get<SubProblem *>("_subproblem");
  THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

  // Coupling
  for (std::set<std::string>::const_iterator iter = parameters.coupledVarsBegin();
       iter != parameters.coupledVarsEnd();
       ++iter)
  {
    std::string name = *iter;
    if (parameters.get<std::vector<std::string> >(*iter) != std::vector<std::string>())
    {
      std::vector<std::string> vars = parameters.get<std::vector<std::string> >(*iter);
      for (unsigned int i = 0; i < vars.size(); i++)
      {
        std::string coupled_var_name = vars[i];
        if (subproblem.hasVariable(coupled_var_name))
          _coupled_vars[name].push_back(&subproblem.getVariable(tid, coupled_var_name));
        else
          mooseError("Coupled variable '" + coupled_var_name + "' was not found\n");
      }
    }
  }
}

unsigned int
Coupleable::getCoupled(const std::string & var_name, unsigned int comp)
{
  return _coupled_vars[var_name][comp]->number();
}

VariableValue &
Coupleable::getCoupledValue(const std::string & var_name, unsigned int comp)
{
  return _coupled_vars[var_name][comp]->sln();
}

VariableValue &
Coupleable::getCoupledNodalValue(const std::string & var_name, unsigned int comp)
{
  return _coupled_vars[var_name][comp]->nodalSln();
}

} // namespace
