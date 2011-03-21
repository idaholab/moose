#include "Coupleable.h"
#include "Moose.h"
#include "Problem.h"
#include "SubProblem.h"

namespace Moose {

Coupleable::Coupleable(InputParameters & parameters)
{
  SubProblem & problem = *parameters.get<SubProblem *>("_problem");
  Problem & topproblem = *problem.parent();

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
        if (topproblem.hasVariable(coupled_var_name))
          _coupled_vars[name].push_back(&topproblem.getVariable(tid, coupled_var_name));
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

VariableGradient &
Coupleable::getCoupledGradient(const std::string & var_name, unsigned int comp)
{
  return _coupled_vars[var_name][comp]->gradSln();
}

VariableValue &
Coupleable::getCoupledNodalValue(const std::string & var_name, unsigned int comp)
{
  return _coupled_vars[var_name][comp]->nodalSln();
}

} // namespace
