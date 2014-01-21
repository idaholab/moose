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
#include "FEProblem.h"

Coupleable::Coupleable(InputParameters & parameters, bool nodal) :
    _nodal(nodal),
    _c_is_implicit(parameters.have_parameter<bool>("implicit") ? parameters.get<bool>("implicit") : true),
    _coupleable_params(parameters)
{
  SubProblem & problem = *parameters.get<SubProblem *>("_subproblem");
  FEProblem & fe_problem = *parameters.get<FEProblem *>("_fe_problem");

  _coupleable_max_qps = fe_problem.getMaxQps();

  THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

  // Coupling
  for (std::set<std::string>::const_iterator iter = parameters.coupledVarsBegin();
       iter != parameters.coupledVarsEnd();
       ++iter)
  {
    std::string name = *iter;
    if (parameters.getVecMooseType(name) != std::vector<std::string>())
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
    else // This means it was optional coupling.  Let's assign a unique id to this variable
      _optional_var_index[name] = std::numeric_limits<unsigned int>::max() - _optional_var_index.size();
  }

  _default_gradient.resize(_coupleable_max_qps);
  _default_second.resize(_coupleable_max_qps);
}

Coupleable::~Coupleable()
{
  for (std::map<std::string, VariableValue *>::iterator it = _default_value.begin(); it != _default_value.end(); ++it)
  {
    it->second->release();
    delete it->second;
  }
  _default_gradient.release();
  _default_second.release();
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
  if(!isCoupled(var_name)) // If it's optionally coupled just return 0
    return _optional_var_index[var_name];

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
  if(!isCoupled(var_name)) // Need to generate a "default value" filled VariableValue
  {
    VariableValue * value = _default_value[var_name];
    if (value == NULL)
    {
      value = new VariableValue(_coupleable_max_qps, _coupleable_params.defaultCoupledValue(var_name));
      _default_value[var_name] = value;
    }
    return *_default_value[var_name];
  }

  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);
  if (_nodal)
    return (_c_is_implicit) ? var->nodalSln() : var->nodalSlnOld();
  else
    return (_c_is_implicit) ? var->sln() : var->slnOld();
}

VariableValue &
Coupleable::coupledValueOld(const std::string & var_name, unsigned int comp)
{
  if(!isCoupled(var_name)) // Need to generate a "default value" filled VariableValue
  {
    VariableValue * value = _default_value[var_name];
    if (value == NULL)
    {
      value = new VariableValue(_coupleable_max_qps, _coupleable_params.defaultCoupledValue(var_name));
      _default_value[var_name] = value;
    }
    return *_default_value[var_name];
  }

  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);
  if (_nodal)
    return (_c_is_implicit) ? var->nodalSlnOld() : var->nodalSlnOlder();
  else
    return (_c_is_implicit) ? var->slnOld() : var->slnOlder();
}

VariableValue &
Coupleable::coupledValueOlder(const std::string & var_name, unsigned int comp)
{
  if(!isCoupled(var_name)) // Need to generate a "default value" filled VariableValue
  {
    VariableValue * value = _default_value[var_name];
    if (value == NULL)
    {
      value = new VariableValue(_coupleable_max_qps, _coupleable_params.defaultCoupledValue(var_name));
      _default_value[var_name] = value;
    }
    return *_default_value[var_name];
  }

  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);
  if (_nodal)
  {
    if (_c_is_implicit)
      return var->nodalSlnOlder();
    else
      mooseError("Older values not available for explicit schemes");
  }
  else
  {
    if (_c_is_implicit)
      return var->slnOlder();
    else
      mooseError("Older values not available for explicit schemes");
  }
}

VariableValue &
Coupleable::coupledDot(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (var->kind() == Moose::VAR_AUXILIARY)
    mooseError("Coupling time derivative of an auxiliary variable is not allowed.");

  if (_nodal)
    return var->nodalSlnDot();
  else
    return var->uDot();
}

VariableValue &
Coupleable::coupledDotDu(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (var->kind() == Moose::VAR_AUXILIARY)
    mooseError("Coupling time derivative of an auxiliary variable is not allowed.");

  if (_nodal)
    return var->nodalSlnDuDotDu();
  else
    return var->duDotDu();
}


VariableGradient &
Coupleable::coupledGradient(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, false);
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSln() : var->gradSlnOld();
}

VariableGradient &
Coupleable::coupledGradientOld(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnOld() : var->gradSlnOlder();
}

VariableGradient &
Coupleable::coupledGradientOlder(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);
  if (_c_is_implicit)
    return var->gradSlnOlder();
  else
    mooseError("Older values not available for explicit schemes");
}

VariableSecond &
Coupleable::coupledSecond(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, false);
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->secondSln() : var->secondSlnOlder();
}

VariableSecond &
Coupleable::coupledSecondOld(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->secondSlnOld() : var->secondSlnOlder();
}

VariableSecond &
Coupleable::coupledSecondOlder(const std::string & var_name, unsigned int comp)
{
  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  MooseVariable * var = getVar(var_name, comp);
  if (_c_is_implicit)
    return var->secondSlnOlder();
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
  MooseVariable * var = getVar(var_name, comp);
  if (_nodal)
    return (_c_is_implicit) ? var->nodalSlnNeighbor() : var->nodalSlnOldNeighbor();
  else
    return (_c_is_implicit) ? var->slnNeighbor() : var->slnOldNeighbor();
}

VariableValue &
NeighborCoupleable::coupledNeighborValueOld(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (_nodal)
    return (_c_is_implicit) ? var->nodalSlnOldNeighbor() : var->nodalSlnOlderNeighbor();
  else
    return (_c_is_implicit) ? var->slnOldNeighbor() : var->slnOlderNeighbor();
}

VariableValue &
NeighborCoupleable::coupledNeighborValueOlder(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (_nodal)
  {
    if (_c_is_implicit)
      return var->nodalSlnOlderNeighbor();
    else
      mooseError("Older values not available for explicit schemes");
  }
  else
  {
    if (_c_is_implicit)
      return var->slnOlderNeighbor();
    else
      mooseError("Older values not available for explicit schemes");
  }
}

VariableGradient &
NeighborCoupleable::coupledNeighborGradient(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

VariableGradient &
NeighborCoupleable::coupledNeighborGradientOld(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

VariableGradient &
NeighborCoupleable::coupledNeighborGradientOlder(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);
  if (_c_is_implicit)
    return var->gradSlnOlderNeighbor();
  else
    mooseError("Older values not available for explicit schemes");
}

VariableSecond &
NeighborCoupleable::coupledNeighborSecond(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->secondSlnNeighbor() : var->secondSlnOldNeighbor();
}
