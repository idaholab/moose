#include "Coupleable.h"
#include "Moose.h"
#include "Problem.h"

Coupleable::Coupleable(InputParameters & parameters)
{
  Moose::Problem & problem = *parameters.get<Moose::Problem *>("_problem");
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
        if (problem.hasVariable(coupled_var_name))
          _coupled_vars[name].push_back(&problem.getVariable(tid, coupled_var_name));
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
