//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Coupleable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "FEProblem.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "InputParameters.h"
#include "MooseObject.h"

Coupleable::Coupleable(const MooseObject * moose_object, bool nodal)
  : _c_parameters(moose_object->parameters()),
    _c_name(_c_parameters.get<std::string>("_object_name")),
    _c_fe_problem(*_c_parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _c_nodal(nodal),
    _c_is_implicit(_c_parameters.have_parameter<bool>("implicit")
                       ? _c_parameters.get<bool>("implicit")
                       : true),
    _c_tid(_c_parameters.get<THREAD_ID>("_tid")),
    _zero(_c_fe_problem._zero[_c_tid]),
    _grad_zero(_c_fe_problem._grad_zero[_c_tid]),
    _second_zero(_c_fe_problem._second_zero[_c_tid]),
    _second_phi_zero(_c_fe_problem._second_phi_zero[_c_tid]),
    _coupleable_neighbor(_c_parameters.have_parameter<bool>("_neighbor")
                             ? _c_parameters.get<bool>("_neighbor")
                             : false),
    _coupleable_max_qps(_c_fe_problem.getMaxQps())
{
  SubProblem & problem = *_c_parameters.getCheckedPointerParam<SubProblem *>("_subproblem");

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
          MooseVariable * moose_var = &problem.getVariable(_c_tid, coupled_var_name);
          _coupled_vars[name].push_back(moose_var);
          _coupled_moose_vars.push_back(moose_var);
        }
        else if (problem.hasScalarVariable(coupled_var_name))
        {
          MooseVariableScalar * moose_scalar_var =
              &problem.getScalarVariable(_c_tid, coupled_var_name);
          _c_coupled_scalar_vars[name].push_back(moose_scalar_var);
        }
        else
          mooseError(_c_name, ": Coupled variable '", coupled_var_name, "' was not found");
      }
    }
    else // This means it was optional coupling.  Let's assign a unique id to this variable
      _optional_var_index[name] =
          std::numeric_limits<unsigned int>::max() - _optional_var_index.size();
  }

  _default_value_zero.resize(_coupleable_max_qps, 0);
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
    if (!_c_parameters.hasCoupledValue(var_name))
      mooseError(_c_name,
                 ": The coupled variable \"",
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

void
Coupleable::checkVar(const std::string & var_name)
{
  auto it = _c_coupled_scalar_vars.find(var_name);
  if (it != _c_coupled_scalar_vars.end())
  {
    std::string cvars;
    for (auto jt : it->second)
      cvars += " " + jt->name();
    mooseError(_c_name,
               ": Trying to couple a scalar variable where field variable is expected, '",
               var_name,
               " =",
               cvars,
               "'");
  }
  // NOTE: non-existent variables are handled in the constructor
}

MooseVariable *
Coupleable::getVar(const std::string & var_name, unsigned int comp)
{
  if (comp < _coupled_vars[var_name].size())
  {
    // Error check - don't couple elemental to nodal
    if (!(_coupled_vars[var_name][comp])->isNodal() && _c_nodal)
      mooseError(_c_name, ": You cannot couple an elemental variable to a nodal variable");
    return _coupled_vars[var_name][comp];
  }
  else
    mooseError(_c_name, ": Trying to get a non-existent component of variable '", var_name, "'");
}

unsigned int
Coupleable::coupled(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);

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
  mooseError(_c_name, ": Unknown variable kind. Corrupted binary?");
}

VariableValue *
Coupleable::getDefaultValue(const std::string & var_name)
{
  std::map<std::string, VariableValue *>::iterator default_value_it = _default_value.find(var_name);
  if (default_value_it == _default_value.end())
  {
    VariableValue * value =
        new VariableValue(_coupleable_max_qps, _c_parameters.defaultCoupledValue(var_name));
    default_value_it = _default_value.insert(std::make_pair(var_name, value)).first;
  }

  return default_value_it->second;
}

const VariableValue &
Coupleable::coupledValue(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name);

  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->nodalSln() : var->nodalSlnOld();
    else
      return (_c_is_implicit) ? var->sln() : var->slnOld();
  }
  else
  {
    if (_c_nodal)
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
  checkVar(var_name);
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name);

  validateExecutionerType(var_name);
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->nodalSlnOld() : var->nodalSlnOlder();
    else
      return (_c_is_implicit) ? var->slnOld() : var->slnOlder();
  }
  else
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->nodalSlnOldNeighbor() : var->nodalSlnOlderNeighbor();
    else
      return (_c_is_implicit) ? var->slnOldNeighbor() : var->slnOlderNeighbor();
  }
}

const VariableValue &
Coupleable::coupledValueOlder(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name);

  validateExecutionerType(var_name);
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
    {
      if (_c_is_implicit)
        return var->nodalSlnOlder();
      else
        mooseError(_c_name, ": Older values not available for explicit schemes");
    }
    else
    {
      if (_c_is_implicit)
        return var->slnOlder();
      else
        mooseError(_c_name, ": Older values not available for explicit schemes");
    }
  }
  else
  {
    if (_c_nodal)
    {
      if (_c_is_implicit)
        return var->nodalSlnOlderNeighbor();
      else
        mooseError(_c_name, ": Older values not available for explicit schemes");
    }
    else
    {
      if (_c_is_implicit)
        return var->slnOlderNeighbor();
      else
        mooseError(_c_name, ": Older values not available for explicit schemes");
    }
  }
}

const VariableValue &
Coupleable::coupledValuePreviousNL(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name);

  _c_fe_problem.needsPreviousNewtonIteration(true);
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->nodalSlnPreviousNL();
    else
      return var->slnPreviousNL();
  }
  else
  {
    if (_c_nodal)
      return var->nodalSlnPreviousNLNeighbor();
    else
      return var->slnPreviousNLNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDot(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->nodalSlnDot();
    else
      return var->uDot();
  }
  else
  {
    if (_c_nodal)
      return var->nodalSlnDotNeighbor();
    else
      return var->uDotNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDotDu(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->nodalSlnDuDotDu();
    else
      return var->duDotDu();
  }
  else
  {
    if (_c_nodal)
      return var->nodalSlnDuDotDu();
    else
      return var->duDotDu();
  }
}

const VariableGradient &
Coupleable::coupledGradient(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  coupledCallback(var_name, false);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSln() : var->gradSlnOld();
  else
    return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have gradients");

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
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have gradients");

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
    mooseError(_c_name, ": Older values not available for explicit schemes");
}

const VariableGradient &
Coupleable::coupledGradientPreviousNL(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  _c_fe_problem.needsPreviousNewtonIteration(true);
  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return var->gradSlnPreviousNL();
  else
    return var->gradSlnPreviousNLNeighbor();
}

const VariableSecond &
Coupleable::coupledSecond(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  coupledCallback(var_name, false);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have second derivatives");

  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->secondSln() : var->secondSlnOlder();
  else
    return (_c_is_implicit) ? var->secondSlnNeighbor() : var->secondSlnOlderNeighbor();
}

const VariableSecond &
Coupleable::coupledSecondOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have second derivatives");

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
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have second derivatives");

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
    mooseError(_c_name, ": Older values not available for explicit schemes");
}

const VariableSecond &
Coupleable::coupledSecondPreviousNL(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  _c_fe_problem.needsPreviousNewtonIteration(true);
  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have second derivatives");

  MooseVariable * var = getVar(var_name, comp);
  if (!_coupleable_neighbor)
    return var->secondSlnPreviousNL();
  else
    return var->secondSlnPreviousNLNeighbor();
}

const VariableValue &
Coupleable::coupledNodalValue(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
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
  checkVar(var_name);
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
  checkVar(var_name);
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
    mooseError(_c_name, ": Older values not available for explicit schemes");
}

const VariableValue &
Coupleable::coupledNodalValuePreviousNL(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
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
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return var->nodalValueDot();
  else
    return var->nodalValueDotNeighbor();
}

const DenseVector<Number> &
Coupleable::coupledSolutionDoFs(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  // default coupling is not available for elemental solutions
  if (!isCoupled(var_name))
    mooseError(_c_name, ": invalid variable name for coupledSolutionDoFs");

  if (_c_nodal)
    mooseError(_c_name, ": nodal objects should not call coupledSolutionDoFs");

  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->solutionDoFs() : var->solutionDoFsOld();
  else
    return (_c_is_implicit) ? var->solutionDoFsNeighbor() : var->solutionDoFsOldNeighbor();
}

const DenseVector<Number> &
Coupleable::coupledSolutionDoFsOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  // default coupling is not available for elemental solutions
  if (!isCoupled(var_name))
    mooseError(_c_name, ": invalid variable name for coupledSolutionDoFsOld");

  if (_c_nodal)
    mooseError(_c_name, ": nodal objects should not call coupledSolutionDoFsOld");

  validateExecutionerType(var_name);
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->solutionDoFsOld() : var->solutionDoFsOlder();
  else
    return (_c_is_implicit) ? var->solutionDoFsOldNeighbor() : var->solutionDoFsOlderNeighbor();
}

const DenseVector<Number> &
Coupleable::coupledSolutionDoFsOlder(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  // default coupling is not available for elemental solutions
  if (!isCoupled(var_name))
    mooseError(_c_name, ": invalid variable name for coupledSolutionDoFsOlder");

  if (_c_nodal)
    mooseError(_c_name, ": nodal objects should not call coupledSolutionDoFsOlder");

  validateExecutionerType(var_name);
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);
  if (_c_is_implicit)
  {
    if (!_coupleable_neighbor)
      return var->solutionDoFsOlder();
    else
      return var->solutionDoFsOlderNeighbor();
  }
  else
    mooseError(_c_name, ": Older values not available for explicit schemes");
}

void
Coupleable::validateExecutionerType(const std::string & name) const
{
  if (!_c_fe_problem.isTransient())
    mooseError(_c_name,
               ": You may not couple in old or older values of \"",
               name,
               "\" when using a \"Steady\" executioner.");
}
