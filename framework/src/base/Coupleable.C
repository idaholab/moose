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
#include "Problem.h"
#include "SubProblem.h"

Coupleable::Coupleable(InputParameters & parameters, bool nodal) :
    _nodal(nodal),
    _c_is_implicit(parameters.have_parameter<bool>("implicit") ? parameters.get<bool>("implicit") : true)
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
        if (problem.hasVariable(coupled_var_name))
        {
          MooseVariable * moose_var = &problem.getVariable(tid, coupled_var_name);
          _coupled_vars[name].push_back(moose_var);
          _coupled_moose_vars.push_back(moose_var);
        }
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

void
Coupleable::coupledCallback(const std::string & /*var_name*/, bool /*is_old*/)
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
  MooseVariable * var = getVar(var_name, comp);
  switch (var->kind())
  {
  case Moose::VAR_NONLINEAR: return getVar(var_name, comp)->index();
  case Moose::VAR_AUXILIARY: return std::numeric_limits<unsigned int>::max() - getVar(var_name, comp)->index();
  }
  mooseError("Unknown variable kind. Corrupted binary?");
}

VariableValue &
Coupleable::coupledValue(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, false);
  if (_nodal)
    return _c_is_implicit ? getVar(var_name, comp)->nodalSln() : getVar(var_name, comp)->nodalSlnOld();
  else
    return _c_is_implicit ? getVar(var_name, comp)->sln() : getVar(var_name, comp)->slnOld();
}

VariableValue &
Coupleable::coupledValueOld(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, true);
  if (_nodal)
    return _c_is_implicit ? getVar(var_name, comp)->nodalSlnOld() : getVar(var_name, comp)->nodalSlnOlder();
  else
    return _c_is_implicit ? getVar(var_name, comp)->slnOld() : getVar(var_name, comp)->slnOlder();
}

VariableValue &
Coupleable::coupledValueOlder(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, true);
  if (_nodal)
  {
    if (_c_is_implicit)
      return getVar(var_name, comp)->nodalSlnOlder();
    else
      mooseError("Older values not available for explicit schemes");
  }
  else
  {
    if (_c_is_implicit)
      return getVar(var_name, comp)->slnOlder();
    else
      mooseError("Older values not available for explicit schemes");
  }
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
  coupledCallback(var_name, false);
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _c_is_implicit ? getVar(var_name, comp)->gradSln() : getVar(var_name, comp)->gradSlnOld();
}

VariableGradient &
Coupleable::coupledGradientOld(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _c_is_implicit ? getVar(var_name, comp)->gradSlnOld() : getVar(var_name, comp)->gradSlnOlder();
}

VariableGradient &
Coupleable::coupledGradientOlder(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  if (_c_is_implicit)
    return getVar(var_name, comp)->gradSlnOlder();
  else
    mooseError("Older values not available for explicit schemes");
}

VariableSecond &
Coupleable::coupledSecond(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, false);
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _c_is_implicit ? getVar(var_name, comp)->secondSln() : getVar(var_name, comp)->secondSlnOlder();
}

VariableSecond &
Coupleable::coupledSecondOld(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _c_is_implicit ? getVar(var_name, comp)->secondSlnOld() : getVar(var_name, comp)->secondSlnOlder();
}

VariableSecond &
Coupleable::coupledSecondOlder(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  if (_c_is_implicit)
    return getVar(var_name, comp)->secondSlnOlder();
  else
    mooseError("Older values not available for explicit schemes");
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
    return _c_is_implicit ? getVar(var_name, comp)->nodalSlnNeighbor() : getVar(var_name, comp)->nodalSlnOldNeighbor();
  else
    return _c_is_implicit ? getVar(var_name, comp)->slnNeighbor() : getVar(var_name, comp)->slnOldNeighbor();
}

VariableValue &
NeighborCoupleable::coupledNeighborValueOld(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    return _c_is_implicit ? getVar(var_name, comp)->nodalSlnOldNeighbor() : getVar(var_name, comp)->nodalSlnOlderNeighbor();
  else
    return _c_is_implicit ? getVar(var_name, comp)->slnOldNeighbor() : getVar(var_name, comp)->slnOlderNeighbor();
}

VariableValue &
NeighborCoupleable::coupledNeighborValueOlder(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
  {
    if (_c_is_implicit)
      return getVar(var_name, comp)->nodalSlnOlderNeighbor();
    else
      mooseError("Older values not available for explicit schemes");
  }
  else
  {
    if (_c_is_implicit)
      return getVar(var_name, comp)->slnOlderNeighbor();
    else
      mooseError("Older values not available for explicit schemes");
  }
}

VariableGradient &
NeighborCoupleable::coupledNeighborGradient(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _c_is_implicit ? getVar(var_name, comp)->gradSlnNeighbor() : getVar(var_name, comp)->gradSlnOldNeighbor();
}

VariableGradient &
NeighborCoupleable::coupledNeighborGradientOld(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _c_is_implicit ? getVar(var_name, comp)->gradSlnOldNeighbor() : getVar(var_name, comp)->gradSlnOlderNeighbor();
}

VariableGradient &
NeighborCoupleable::coupledNeighborGradientOlder(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  if (_c_is_implicit)
    return getVar(var_name, comp)->gradSlnOlderNeighbor();
  else
    mooseError("Older values not available for explicit schemes");
}

VariableSecond &
NeighborCoupleable::coupledNeighborSecond(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _c_is_implicit ? getVar(var_name, comp)->secondSlnNeighbor() : getVar(var_name, comp)->secondSlnOldNeighbor();
}


// Scalar

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
  return _sc_is_implicit ? getScalarVar(var_name, comp)->sln() : getScalarVar(var_name, comp)->slnOld();
}

VariableValue &
ScalarCoupleable::coupledScalarValueOld(const std::string & var_name, unsigned int comp)
{
  return _sc_is_implicit ? getScalarVar(var_name, comp)->slnOld() : getScalarVar(var_name, comp)->slnOlder();
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
