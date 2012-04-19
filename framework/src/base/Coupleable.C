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
#include "SubProblem.h"

Coupleable::Coupleable(InputParameters & parameters, bool nodal) :
    _nodal(nodal)
{
  SubProblem & problem = *parameters.get<SubProblem *>("_subproblem");

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
        else if (problem.hasScalarVariable(coupled_var_name))
          ; // ignore scalar variables
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
    {
      // Error check - don't couple elemental to nodal
      if (!(_coupled_vars[var_name][comp])->isNodal() && _nodal)
        mooseError("You cannot couple an elemental variable to a nodal variable");
      return _coupled_vars[var_name][comp];
    }
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
  if (getVar(var_name, comp)->kind() == Moose::VAR_AUXILIARY)
    mooseError("Coupling time derivative of an auxiliary variable is not allowed.");

  if (_nodal)
    return getVar(var_name, comp)->nodalSlnDot();
  else
    return getVar(var_name, comp)->uDot();
}

VariableValue &
Coupleable::coupledDotDu(const std::string & var_name, unsigned int comp)
{
  if (getVar(var_name, comp)->kind() == Moose::VAR_AUXILIARY)
    mooseError("Coupling time derivative of an auxiliary variable is not allowed.");

  if (_nodal)
    return getVar(var_name, comp)->nodalSlnDuDotDu();
  else
    return getVar(var_name, comp)->duDotDu();
}


VariableGradient &
Coupleable::coupledGradient(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return getVar(var_name, comp)->gradSln();
}

VariableGradient &
Coupleable::coupledGradientOld(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return getVar(var_name, comp)->gradSlnOld();
}

VariableGradient &
Coupleable::coupledGradientOlder(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return getVar(var_name, comp)->gradSlnOlder();
}

VariableSecond &
Coupleable::coupledSecond(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return getVar(var_name, comp)->secondSln();
}

VariableSecond &
Coupleable::coupledSecondOld(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return getVar(var_name, comp)->secondSlnOld();
}

VariableSecond &
Coupleable::coupledSecondOlder(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return getVar(var_name, comp)->secondSlnOlder();
}


// Neighbor values

NeighborCoupleable::NeighborCoupleable(InputParameters & parameters, bool nodal) :
    Coupleable(parameters, nodal)
{
}

NeighborCoupleable::~NeighborCoupleable()
{
}


VariableValue &
NeighborCoupleable::coupledNeighborValue(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    return getVar(var_name, comp)->nodalSlnNeighbor();
  else
    return getVar(var_name, comp)->slnNeighbor();
}

VariableValue &
NeighborCoupleable::coupledNeighborValueOld(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    return getVar(var_name, comp)->nodalSlnOldNeighbor();
  else
    return getVar(var_name, comp)->slnOldNeighbor();
}

VariableValue &
NeighborCoupleable::coupledNeighborValueOlder(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    return getVar(var_name, comp)->nodalSlnOlderNeighbor();
  else
    return getVar(var_name, comp)->slnOlderNeighbor();
}

VariableGradient &
NeighborCoupleable::coupledNeighborGradient(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return getVar(var_name, comp)->gradSlnNeighbor();
}

VariableGradient &
NeighborCoupleable::coupledNeighborGradientOld(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return getVar(var_name, comp)->gradSlnOldNeighbor();
}

VariableGradient &
NeighborCoupleable::coupledNeighborGradientOlder(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return getVar(var_name, comp)->gradSlnOlderNeighbor();
}

VariableSecond &
NeighborCoupleable::coupledNeighborSecond(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return getVar(var_name, comp)->secondSlnNeighbor();
}


// Scalar

ScalarCoupleable::ScalarCoupleable(InputParameters & parameters)
{
  SubProblem & problem = *parameters.get<SubProblem *>("_subproblem");

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
        if (problem.hasScalarVariable(coupled_var_name))
          _coupled_scalar_vars[name].push_back(&problem.getScalarVariable(tid, coupled_var_name));
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
  return getScalarVar(var_name, comp)->number();
}


VariableValue &
ScalarCoupleable::coupledScalarValue(const std::string & var_name, unsigned int comp)
{
  return getScalarVar(var_name, comp)->sln();
}

VariableValue &
ScalarCoupleable::coupledScalarValueOld(const std::string & var_name, unsigned int comp)
{
  return getScalarVar(var_name, comp)->slnOld();
}

VariableValue &
ScalarCoupleable::coupledScalarDot(const std::string & var_name, unsigned int comp)
{
  if (getScalarVar(var_name, comp)->kind() == Moose::VAR_AUXILIARY)
    mooseError("Coupling time derivative of an auxiliary variable is not allowed.");

  return getScalarVar(var_name, comp)->uDot();
}

VariableValue &
ScalarCoupleable::coupledScalarDotDu(const std::string & var_name, unsigned int comp)
{
  if (getScalarVar(var_name, comp)->kind() == Moose::VAR_AUXILIARY)
    mooseError("Coupling time derivative of an auxiliary variable is not allowed.");

  return getScalarVar(var_name, comp)->duDotDu();
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
