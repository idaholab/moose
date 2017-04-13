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
#include "MooseVariable.h"
#include "InputParameters.h"
#include "MooseObject.h"

Coupleable::Coupleable(const MooseObject * moose_object, bool nodal)
  : _c_parameters(moose_object->parameters()),
    _c_fe_problem(*_c_parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _nodal(nodal),
    _c_is_implicit(_c_parameters.have_parameter<bool>("implicit")
                       ? _c_parameters.get<bool>("implicit")
                       : true),
    _coupleable_params(_c_parameters),
    _coupleable_neighbor(_c_parameters.have_parameter<bool>("_neighbor")
                             ? _c_parameters.get<bool>("_neighbor")
                             : false),
    _coupleable_max_qps(_c_fe_problem.getMaxQps())
{
  SubProblem & problem = *_c_parameters.get<SubProblem *>("_subproblem");

  THREAD_ID tid = _c_parameters.get<THREAD_ID>("_tid");

  // Coupling
  for (std::set<std::string>::const_iterator iter = _c_parameters.coupledVarsBegin();
       iter != _c_parameters.coupledVarsEnd();
       ++iter)
  {
    std::string name = *iter;
    if (_c_parameters.getVecMooseType(name) != std::vector<std::string>())
    {
      std::vector<std::string> vars = _c_parameters.getVecMooseType(*iter);
      for (const auto & coupled_var_name : vars)
      {
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
      _optional_var_index[name] =
          std::numeric_limits<unsigned int>::max() - _optional_var_index.size();
  }

  _default_value_zero.resize(_coupleable_max_qps);
  _default_gradient.resize(_coupleable_max_qps);
  _default_second.resize(_coupleable_max_qps);
}

Coupleable::~Coupleable()
{
  for (auto & it : _default_value)
  {
    it.second->release();
    delete it.second;
  }
  _default_value_zero.release();
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
  std::map<std::string, std::vector<MooseVariable *>>::iterator it = _coupled_vars.find(var_name);
  if (it != _coupled_vars.end())
    return (i < it->second.size());
  else
  {
    // Make sure the user originally requested this value in the InputParameter syntax
    if (!_coupleable_params.hasCoupledValue(var_name))
      mooseError("The coupled variable \"",
                 var_name,
                 "\" was never added to this objects's "
                 "InputParameters, please double-check your "
                 "spelling");

    return false;
  }
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
    case Moose::VAR_NONLINEAR:
      return var->number();
    case Moose::VAR_AUXILIARY:
      return std::numeric_limits<unsigned int>::max() - var->number();
  }
  mooseError("Unknown variable kind. Corrupted binary?");
}

VariableValue *
Coupleable::getDefaultValue(const std::string & var_name)
{
  std::map<std::string, VariableValue *>::iterator default_value_it = _default_value.find(var_name);
  if (default_value_it == _default_value.end())
  {
    VariableValue * value =
        new VariableValue(_coupleable_max_qps, _coupleable_params.defaultCoupledValue(var_name));
    default_value_it = _default_value.insert(std::make_pair(var_name, value)).first;
  }

  return default_value_it->second;
}

const VariableValue &
Coupleable::coupledValue(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name);

  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
    if (_nodal)
      return (_c_is_implicit) ? var->nodalSln() : var->nodalSlnOld();
    else
      return (_c_is_implicit) ? var->sln() : var->slnOld();
  }
  else
  {
    if (_nodal)
      return (_c_is_implicit) ? var->nodalSlnNeighbor() : var->nodalSlnOldNeighbor();
    else
      return (_c_is_implicit) ? var->slnNeighbor() : var->slnOldNeighbor();
  }
}

VariableValue &
Coupleable::writableCoupledValue(const std::string & var_name, unsigned int comp)
{
  return const_cast<VariableValue &>(coupledValue(var_name, comp));
}

const VariableValue &
Coupleable::coupledValueOld(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name);

  validateExecutionerType(var_name);
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
    if (_nodal)
      return (_c_is_implicit) ? var->nodalSlnOld() : var->nodalSlnOlder();
    else
      return (_c_is_implicit) ? var->slnOld() : var->slnOlder();
  }
  else
  {
    if (_nodal)
      return (_c_is_implicit) ? var->nodalSlnOldNeighbor() : var->nodalSlnOlderNeighbor();
    else
      return (_c_is_implicit) ? var->slnOldNeighbor() : var->slnOlderNeighbor();
  }
}

const VariableValue &
Coupleable::coupledValueOlder(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name);

  validateExecutionerType(var_name);
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
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
  else
  {
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
}

const VariableValue &
Coupleable::coupledValuePreviousNL(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name);

  _c_fe_problem.needsPreviousNewtonIteration(true);
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
    if (_nodal)
      return var->nodalSlnPreviousNL();
    else
      return var->slnPreviousNL();
  }
  else
  {
    if (_nodal)
      return var->nodalSlnPreviousNLNeighbor();
    else
      return var->slnPreviousNLNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDot(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
    if (_nodal)
      return var->nodalSlnDot();
    else
      return var->uDot();
  }
  else
  {
    if (_nodal)
      return var->nodalSlnDotNeighbor();
    else
      return var->uDotNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDotDu(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
    if (_nodal)
      return var->nodalSlnDuDotDu();
    else
      return var->duDotDu();
  }
  else
  {
    if (_nodal)
      return var->nodalSlnDuDotDu();
    else
      return var->duDotDu();
  }
}

const VariableGradient &
Coupleable::coupledGradient(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  coupledCallback(var_name, false);
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSln() : var->gradSlnOld();
  else
    return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientOld(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  validateExecutionerType(var_name);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSlnOld() : var->gradSlnOlder();
  else
    return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientOlder(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  validateExecutionerType(var_name);
  MooseVariable * var = getVar(var_name, comp);

  if (_c_is_implicit)
  {
    if (!_coupleable_neighbor)
      return var->gradSlnOlder();
    else
      return var->gradSlnOlderNeighbor();
  }
  else
    mooseError("Older values not available for explicit schemes");
}

const VariableGradient &
Coupleable::coupledGradientPreviousNL(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  _c_fe_problem.needsPreviousNewtonIteration(true);
  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return var->gradSlnPreviousNL();
  else
    return var->gradSlnPreviousNLNeighbor();
}

const VariableSecond &
Coupleable::coupledSecond(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  coupledCallback(var_name, false);
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->secondSln() : var->secondSlnOlder();
  else
    return (_c_is_implicit) ? var->secondSlnNeighbor() : var->secondSlnOlderNeighbor();
}

const VariableSecond &
Coupleable::coupledSecondOld(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  validateExecutionerType(var_name);
  MooseVariable * var = getVar(var_name, comp);
  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->secondSlnOld() : var->secondSlnOlder();
  else
    return (_c_is_implicit) ? var->secondSlnOldNeighbor() : var->secondSlnOlderNeighbor();
}

const VariableSecond &
Coupleable::coupledSecondOlder(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  validateExecutionerType(var_name);
  MooseVariable * var = getVar(var_name, comp);
  if (_c_is_implicit)
  {
    if (!_coupleable_neighbor)
      return var->secondSlnOlder();
    else
      return var->secondSlnOlderNeighbor();
  }
  else
    mooseError("Older values not available for explicit schemes");
}

const VariableSecond &
Coupleable::coupledSecondPreviousNL(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  _c_fe_problem.needsPreviousNewtonIteration(true);
  coupledCallback(var_name, true);
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  MooseVariable * var = getVar(var_name, comp);
  if (!_coupleable_neighbor)
    return var->secondSlnPreviousNL();
  else
    return var->secondSlnPreviousNLNeighbor();
}

const VariableValue &
Coupleable::coupledNodalValue(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name);

  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->nodalValue() : var->nodalValueOld();
  else
    return (_c_is_implicit) ? var->nodalValueNeighbor() : var->nodalValueOldNeighbor();
}

const VariableValue &
Coupleable::coupledNodalValueOld(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name);

  validateExecutionerType(var_name);
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->nodalValueOld() : var->nodalValueOlder();
  else
    return (_c_is_implicit) ? var->nodalValueOldNeighbor() : var->nodalValueOlderNeighbor();
}

const VariableValue &
Coupleable::coupledNodalValueOlder(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name);

  validateExecutionerType(var_name);
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);
  if (_c_is_implicit)
  {
    if (!_coupleable_neighbor)
      return var->nodalValueOlder();
    else
      return var->nodalValueOlderNeighbor();
  }
  else
    mooseError("Older values not available for explicit schemes");
}

const VariableValue &
Coupleable::coupledNodalValuePreviousNL(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name);

  _c_fe_problem.needsPreviousNewtonIteration(true);
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return var->nodalValuePreviousNL();
  else
    return var->nodalValuePreviousNLNeighbor();
}

const VariableValue &
Coupleable::coupledNodalDot(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return var->nodalValueDot();
  else
    return var->nodalValueDotNeighbor();
}

void
Coupleable::validateExecutionerType(const std::string & name) const
{
  if (!_c_fe_problem.isTransient())
    mooseError("You may not couple in old or older values of \"",
               name,
               "\" when using a \"Steady\" executioner.");
}
