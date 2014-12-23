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
    _c_fe_problem(*parameters.getCheckedPointerParam<FEProblem *>("_fe_problem")),
    _nodal(nodal),
    _c_is_implicit(parameters.have_parameter<bool>("implicit") ? parameters.get<bool>("implicit") : true),
    _coupleable_params(parameters)
{
  SubProblem & problem = *parameters.get<SubProblem *>("_subproblem");

  _coupleable_max_qps = _c_fe_problem.getMaxQps();

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

  _default_value_zero.resize(_coupleable_max_qps);
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
  _default_value_zero.release();
  _default_gradient.release();
  _default_second.release();
}

void
Coupleable::coupledCallback(MooseVariable * /*var*/, bool /*is_old*/)
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
  if (!isCoupled(var_name))
    return _optional_var_index[var_name];

  MooseVariable * var = getVar(var_name, comp);
  switch (var->kind())
  {
  case Moose::VAR_NONLINEAR: return getVar(var_name, comp)->number();
  case Moose::VAR_AUXILIARY: return std::numeric_limits<unsigned int>::max() - getVar(var_name, comp)->number();
  }
  mooseError("Unknown variable kind. Corrupted binary?");
}

VariableValue &
Coupleable::coupledValue(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Need to generate a "default value" filled VariableValue
  {
    VariableValue * value = _default_value[var_name];
    if (value == NULL)
    {
      value = new VariableValue(_coupleable_max_qps, _coupleable_params.defaultCoupledValue(var_name));
      _default_value[var_name] = value;
    }
    return *_default_value[var_name];
  }

  return coupledValue(getVar(var_name, comp));
}

VariableValue &
Coupleable::coupledValue(MooseVariable * var)
{
  coupledCallback(var, false);

  if (_nodal)
    return (_c_is_implicit) ? var->nodalSln() : var->nodalSlnOld();
  else
    return (_c_is_implicit) ? var->sln() : var->slnOld();
}

VariableValue &
Coupleable::coupledValueOld(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Need to generate a "default value" filled VariableValue
  {
    VariableValue * value = _default_value[var_name];
    if (value == NULL)
    {
      value = new VariableValue(_coupleable_max_qps, _coupleable_params.defaultCoupledValue(var_name));
      _default_value[var_name] = value;
    }
    return *_default_value[var_name];
  }

  return coupledValueOld(getVar(var_name, comp));
}

VariableValue &
Coupleable::coupledValueOld(MooseVariable * var)
{
  validateExecutionerType(var);
  coupledCallback(var, true);

  if (_nodal)
    return (_c_is_implicit) ? var->nodalSlnOld() : var->nodalSlnOlder();
  else
    return (_c_is_implicit) ? var->slnOld() : var->slnOlder();
}

VariableValue &
Coupleable::coupledValueOlder(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Need to generate a "default value" filled VariableValue
  {
    VariableValue * value = _default_value[var_name];
    if (value == NULL)
    {
      value = new VariableValue(_coupleable_max_qps, _coupleable_params.defaultCoupledValue(var_name));
      _default_value[var_name] = value;
    }
    return *_default_value[var_name];
  }

  return coupledValueOlder(getVar(var_name, comp));
}

VariableValue &
Coupleable::coupledValueOlder(MooseVariable * var)
{
  validateExecutionerType(var);
  coupledCallback(var, true);

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
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  return coupledDot(getVar(var_name, comp));
}

VariableValue &
Coupleable::coupledDot(MooseVariable * var)
{
  if (_nodal)
    return var->nodalSlnDot();
  else
    return var->uDot();
}

VariableValue &
Coupleable::coupledDotDu(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  return coupledDotDu(getVar(var_name, comp));
}

VariableValue &
Coupleable::coupledDotDu(MooseVariable * var)
{
  if (_nodal)
    return var->nodalSlnDuDotDu();
  else
    return var->duDotDu();
}


VariableGradient &
Coupleable::coupledGradient(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  return coupledGradient(getVar(var_name, comp));
}

VariableGradient &
Coupleable::coupledGradient(MooseVariable * var)
{
  coupledCallback(var, false);

  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return (_c_is_implicit) ? var->gradSln() : var->gradSlnOld();
}

VariableGradient &
Coupleable::coupledGradientOld(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;
  return coupledGradientOld(getVar(var_name, comp));
}

VariableGradient &
Coupleable::coupledGradientOld(MooseVariable * var)
{
  validateExecutionerType(var);
  coupledCallback(var, true);

  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return (_c_is_implicit) ? var->gradSlnOld() : var->gradSlnOlder();
}

VariableGradient &
Coupleable::coupledGradientOlder(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  return coupledGradientOlder(getVar(var_name, comp));
}

VariableGradient &
Coupleable::coupledGradientOlder(MooseVariable * var)
{
  validateExecutionerType(var);
  coupledCallback(var, true);

  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  if (_c_is_implicit)
    return var->gradSlnOlder();
  else
    mooseError("Older values not available for explicit schemes");
}


VariableSecond &
Coupleable::coupledSecond(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  return coupledSecond(getVar(var_name, comp));
}

VariableSecond &
Coupleable::coupledSecond(MooseVariable * var)
{
  coupledCallback(var, false);

  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return (_c_is_implicit) ? var->secondSln() : var->secondSlnOlder();
}

VariableSecond &
Coupleable::coupledSecondOld(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  return coupledSecondOld(getVar(var_name, comp));
}

VariableSecond &
Coupleable::coupledSecondOld(MooseVariable * var)
{
  validateExecutionerType(var);
  coupledCallback(var, true);

  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return (_c_is_implicit) ? var->secondSlnOld() : var->secondSlnOlder();
}

VariableSecond &
Coupleable::coupledSecondOlder(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  return coupledSecondOlder(getVar(var_name, comp));
}

VariableSecond &
Coupleable::coupledSecondOlder(MooseVariable * var)
{
  validateExecutionerType(var);
  coupledCallback(var, true);

  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  if (_c_is_implicit)
    return var->secondSlnOlder();
  else
    mooseError("Older values not available for explicit schemes");
}


VariableValue &
Coupleable::coupledNodalValue(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Need to generate a "default value" filled VariableValue
  {
    VariableValue * value = _default_value[var_name];
    if (value == NULL)
    {
      value = new VariableValue(_coupleable_max_qps, _coupleable_params.defaultCoupledValue(var_name));
      _default_value[var_name] = value;
    }
    return *_default_value[var_name];
  }

  return coupledNodalValue(getVar(var_name, comp));
}

VariableValue &
Coupleable::coupledNodalValue(MooseVariable * var)
{
  coupledCallback(var, false);
  return (_c_is_implicit) ? var->nodalValue() : var->nodalValueOld();
}

VariableValue &
Coupleable::coupledNodalValueOld(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Need to generate a "default value" filled VariableValue
  {
    VariableValue * value = _default_value[var_name];
    if (value == NULL)
    {
      value = new VariableValue(_coupleable_max_qps, _coupleable_params.defaultCoupledValue(var_name));
      _default_value[var_name] = value;
    }
    return *_default_value[var_name];
  }

  return coupledNodalValueOld(getVar(var_name, comp));
}

VariableValue &
Coupleable::coupledNodalValueOld(MooseVariable * var)
{
  validateExecutionerType(var);
  coupledCallback(var, true);
  return (_c_is_implicit) ? var->nodalValueOld() : var->nodalValueOlder();
}

VariableValue &
Coupleable::coupledNodalValueOlder(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Need to generate a "default value" filled VariableValue
  {
    VariableValue * value = _default_value[var_name];
    if (value == NULL)
    {
      value = new VariableValue(_coupleable_max_qps, _coupleable_params.defaultCoupledValue(var_name));
      _default_value[var_name] = value;
    }
    return *_default_value[var_name];
  }

  return coupledNodalValueOlder(getVar(var_name, comp));
}

VariableValue &
Coupleable::coupledNodalValueOlder(MooseVariable * var)
{
  validateExecutionerType(var);
  coupledCallback(var, true);

  if (_c_is_implicit)
    return var->nodalValueOlder();
  else
    mooseError("Older values not available for explicit schemes");
}

void
Coupleable::validateExecutionerType(MooseVariable * var) const
{
  if (!_c_fe_problem.isTransient())
    mooseError("You may not couple in old or older values of \"" << var->name() << "\" when using a \"Steady\" executioner.");
}
