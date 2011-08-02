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

#include "Coupleable.h"
#include "Moose.h"
#include "Problem.h"
#include "SubProblemInterface.h"

Coupleable::Coupleable(InputParameters & parameters, bool nodal) :
    _nodal(nodal)
{
  SubProblemInterface & problem = *parameters.get<SubProblemInterface *>("_subproblem");
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

Coupleable::~Coupleable()
{
}

bool
Coupleable::isCoupled(const std::string & var_name, unsigned int i)
{
  std::map<std::string, std::vector<MooseVariable *> >::iterator it = _coupled_vars.find(var_name);
  if (it != _coupled_vars.end())
    return (i < it->second.size());
  else
    return false;
}

unsigned int
Coupleable::coupledComponents(const std::string & var_name)
{
  return _coupled_vars[var_name].size();
}

MooseVariable *
Coupleable::getVar(const std::string & var_name, unsigned int comp)
{
  if (_coupled_vars.find(var_name) != _coupled_vars.end())
  {
    if (comp < _coupled_vars[var_name].size())
      return _coupled_vars[var_name][comp];
    else
      mooseError("Trying to get a non-existent component of variable '" + var_name + "'");
  }
  else
    mooseError("Trying to get a non-existent variable '" + var_name + "'");
}

unsigned int
Coupleable::coupled(const std::string & var_name, unsigned int comp)
{
  return getVar(var_name, comp)->number();
}

VariableValue &
Coupleable::coupledValue(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    return getVar(var_name, comp)->nodalSln();
  else
    return getVar(var_name, comp)->sln();
}

VariableValue &
Coupleable::coupledValueOld(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    return getVar(var_name, comp)->nodalSlnOld();
  else
    return getVar(var_name, comp)->slnOld();
}

VariableValue &
Coupleable::coupledValueOlder(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    return getVar(var_name, comp)->nodalSlnOlder();
  else
    return getVar(var_name, comp)->slnOlder();
}

VariableValue &
Coupleable::coupledDot(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have u_dot defined");
  else
    return getVar(var_name, comp)->uDot();
}

VariableValue &
Coupleable::coupledDotDu(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have du_dot_du defined");
  else
    return getVar(var_name, comp)->duDotDu();
}


VariableGradient &
Coupleable::coupledGradient(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");
  else
    return getVar(var_name, comp)->gradSln();
}

VariableGradient &
Coupleable::coupledGradientOld(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");
  else
    return getVar(var_name, comp)->gradSlnOld();
}

VariableGradient &
Coupleable::coupledGradientOlder(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");
  else
    return getVar(var_name, comp)->gradSlnOlder();
}

VariableSecond &
Coupleable::coupledSecond(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");
  else
    return getVar(var_name, comp)->secondSln();
}

VariableSecond &
Coupleable::coupledSecondOld(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");
  else
    return getVar(var_name, comp)->secondSlnOld();
}

VariableSecond &
Coupleable::coupledSecondOlder(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");
  else
    return getVar(var_name, comp)->secondSlnOlder();
}
