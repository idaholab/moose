/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ScalarCoupleable.h"
#include "Problem.h"
#include "SubProblem.h"

ScalarCoupleable::ScalarCoupleable(InputParameters & parameters) :
    _sc_is_implicit(parameters.have_parameter<bool>("implicit") ? parameters.get<bool>("implicit") : true)
{

  SubProblem & problem = *parameters.get<SubProblem *>("_subproblem");

  THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

  // Coupling
  for (std::set<std::string>::const_iterator iter = parameters.coupledVarsBegin();
       iter != parameters.coupledVarsEnd();
       ++iter)
  {
    std::string name = *iter;
    if (parameters.getVecMooseType(*iter) != std::vector<std::string>())
    {
      std::vector<std::string> vars = parameters.getVecMooseType(*iter);
      for (unsigned int i = 0; i < vars.size(); i++)
      {
        std::string coupled_var_name = vars[i];
        if (problem.hasScalarVariable(coupled_var_name))
        {
          MooseVariableScalar * scalar_var = &problem.getScalarVariable(tid, coupled_var_name);
          _coupled_scalar_vars[name].push_back(scalar_var);
          _coupled_moose_scalar_vars.push_back(scalar_var);
        }
        else if (problem.hasVariable(coupled_var_name))
          ; // ignore normal variables
        else
          mooseError("Coupled variable '" + coupled_var_name + "' was not found\n");
      }
    }
  }
}

ScalarCoupleable::~ScalarCoupleable()
{
}

const std::vector<MooseVariableScalar *> &
ScalarCoupleable::getCoupledMooseScalarVars()
{
  return _coupled_moose_scalar_vars;
}

bool
ScalarCoupleable::isCoupledScalar(const std::string & var_name, unsigned int i)
{
  std::map<std::string, std::vector<MooseVariableScalar *> >::iterator it = _coupled_scalar_vars.find(var_name);
  if (it != _coupled_scalar_vars.end())
    return (i < it->second.size());
  else
    return false;
}

unsigned int
ScalarCoupleable::coupledScalar(const std::string & var_name, unsigned int comp)
{
  return getScalarVar(var_name, comp)->index();
}


VariableValue &
ScalarCoupleable::coupledScalarValue(const std::string & var_name, unsigned int comp)
{
  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return (_sc_is_implicit) ? var->sln() : var->slnOld();
}

VariableValue &
ScalarCoupleable::coupledScalarValueOld(const std::string & var_name, unsigned int comp)
{
  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return (_sc_is_implicit) ? var->slnOld() : var->slnOlder();
}

VariableValue &
ScalarCoupleable::coupledScalarDot(const std::string & var_name, unsigned int comp)
{
  MooseVariableScalar * var = getScalarVar(var_name, comp);
  if (var->kind() == Moose::VAR_AUXILIARY)
    mooseError("Coupling time derivative of an auxiliary variable is not allowed.");

  return var->uDot();
}

VariableValue &
ScalarCoupleable::coupledScalarDotDu(const std::string & var_name, unsigned int comp)
{
  MooseVariableScalar * var = getScalarVar(var_name, comp);
  if (var->kind() == Moose::VAR_AUXILIARY)
    mooseError("Coupling time derivative of an auxiliary variable is not allowed.");

  return var->duDotDu();
}


MooseVariableScalar *
ScalarCoupleable::getScalarVar(const std::string & var_name, unsigned int comp)
{
  if (_coupled_scalar_vars.find(var_name) != _coupled_scalar_vars.end())
  {
    if (comp < _coupled_scalar_vars[var_name].size())
      return _coupled_scalar_vars[var_name][comp];
    else
      mooseError("Trying to get a non-existent component of variable '" + var_name + "'");
  }
  else
    mooseError("Trying to get a non-existent variable '" + var_name + "'");
}

unsigned int
ScalarCoupleable::coupledScalarComponents(const std::string & var_name)
{
  return _coupled_scalar_vars[var_name].size();
}
