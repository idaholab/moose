//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarCoupleable.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseVariableFEBase.h"
#include "MooseVariableScalar.h"
#include "Problem.h"
#include "SubProblem.h"

ScalarCoupleable::ScalarCoupleable(const MooseObject * moose_object)
  : _sc_parameters(moose_object->parameters()),
    _sc_name(_sc_parameters.get<std::string>("_object_name")),
    _sc_fe_problem(*_sc_parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sc_is_implicit(_sc_parameters.have_parameter<bool>("implicit")
                        ? _sc_parameters.get<bool>("implicit")
                        : true),
    _coupleable_params(_sc_parameters),
    _sc_tid(_sc_parameters.isParamValid("_tid") ? _sc_parameters.get<THREAD_ID>("_tid") : 0),
    _real_zero(_sc_fe_problem._real_zero[_sc_tid]),
    _scalar_zero(_sc_fe_problem._scalar_zero[_sc_tid]),
    _point_zero(_sc_fe_problem._point_zero[_sc_tid])
{
  SubProblem & problem = *_sc_parameters.getCheckedPointerParam<SubProblem *>("_subproblem");

  // Coupling
  for (std::set<std::string>::const_iterator iter = _sc_parameters.coupledVarsBegin();
       iter != _sc_parameters.coupledVarsEnd();
       ++iter)
  {
    std::string name = *iter;
    if (_sc_parameters.getVecMooseType(*iter) != std::vector<std::string>())
    {
      std::vector<std::string> vars = _sc_parameters.getVecMooseType(*iter);
      for (const auto & coupled_var_name : vars)
      {
        if (problem.hasScalarVariable(coupled_var_name))
        {
          MooseVariableScalar * scalar_var = &problem.getScalarVariable(_sc_tid, coupled_var_name);
          _coupled_scalar_vars[name].push_back(scalar_var);
          _coupled_moose_scalar_vars.push_back(scalar_var);
        }
        else if (problem.hasVariable(coupled_var_name))
        {
          MooseVariableFEBase * moose_var =
              &problem.getVariable(_sc_tid,
                                   coupled_var_name,
                                   Moose::VarKindType::VAR_ANY,
                                   Moose::VarFieldType::VAR_FIELD_ANY);
          _sc_coupled_vars[name].push_back(moose_var);
        }
        else
          mooseError(_sc_name, ": Coupled variable '", coupled_var_name, "' was not found");
      }
    }
  }
}

const std::vector<MooseVariableScalar *> &
ScalarCoupleable::getCoupledMooseScalarVars()
{
  return _coupled_moose_scalar_vars;
}

bool
ScalarCoupleable::isCoupledScalar(const std::string & var_name, unsigned int i)
{
  auto it = _coupled_scalar_vars.find(var_name);
  if (it != _coupled_scalar_vars.end())
    return (i < it->second.size());
  else
  {
    // Make sure the user originally requested this value in the InputParameter syntax
    if (!_coupleable_params.hasCoupledValue(var_name))
      mooseError(_sc_name,
                 ": The coupled scalar variable \"",
                 var_name,
                 "\" was never added to this objects's "
                 "InputParameters, please double-check "
                 "your spelling");

    return false;
  }
}

unsigned int
ScalarCoupleable::coupledScalar(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  return getScalarVar(var_name, comp)->number();
}

Order
ScalarCoupleable::coupledScalarOrder(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupledScalar(var_name, comp))
    return _sc_fe_problem.getMaxScalarOrder();

  return getScalarVar(var_name, comp)->order();
}

VariableValue *
ScalarCoupleable::getDefaultValue(const std::string & var_name)
{
  auto default_value_it = _default_value.find(var_name);
  if (default_value_it == _default_value.end())
  {
    auto value = libmesh_make_unique<VariableValue>(
        _sc_fe_problem.getMaxScalarOrder(), _coupleable_params.defaultCoupledValue(var_name));
    default_value_it = _default_value.insert(std::make_pair(var_name, std::move(value))).first;
  }

  return default_value_it->second.get();
}

VariableValue &
ScalarCoupleable::coupledScalarValue(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupledScalar(var_name, comp))
    return *getDefaultValue(var_name);

  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return (_sc_is_implicit) ? var->sln() : var->slnOld();
}

const ADVariableValue &
ScalarCoupleable::adCoupledScalarValue(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupledScalar(var_name, comp))
    return *getADDefaultValue(var_name);

  MooseVariableScalar * var = getScalarVar(var_name, comp);

  if (_sc_is_implicit)
    return var->adSln();
  else
    mooseError("adCoupledValue for non-implicit calculations is not currently supported. Use "
               "coupledValue instead for non-implicit");
}

template <>
const GenericVariableValue<false> &
ScalarCoupleable::coupledGenericScalarValue<false>(const std::string & var_name, unsigned int comp)
{
  return coupledScalarValue(var_name, comp);
}

template <>
const GenericVariableValue<true> &
ScalarCoupleable::coupledGenericScalarValue<true>(const std::string & var_name, unsigned int comp)
{
  return adCoupledScalarValue(var_name, comp);
}

ADVariableValue *
ScalarCoupleable::getADDefaultValue(const std::string & var_name)
{
  auto default_value_it = _dual_default_value.find(var_name);
  if (default_value_it == _dual_default_value.end())
  {
    auto value = libmesh_make_unique<ADVariableValue>(
        _sc_fe_problem.getMaxScalarOrder(), _coupleable_params.defaultCoupledValue(var_name));
    default_value_it = _dual_default_value.insert(std::make_pair(var_name, std::move(value))).first;
  }

  return default_value_it->second.get();
}

VariableValue &
ScalarCoupleable::coupledVectorTagScalarValue(const std::string & var_name,
                                              TagID tag,
                                              unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupledScalar(var_name, comp))
    return *getDefaultValue(var_name);

  if (!_sc_fe_problem.vectorTagExists(tag))
    mooseError("Attempting to couple to vector tag scalar with ID ",
               tag,
               "in ",
               _sc_name,
               ", but a vector tag with that ID does not exist");

  addScalarVariableCoupleableVectorTag(tag);

  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return var->vectorTagSln(tag);
}

VariableValue &
ScalarCoupleable::coupledMatrixTagScalarValue(const std::string & var_name,
                                              TagID tag,
                                              unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupledScalar(var_name, comp))
    return *getDefaultValue(var_name);

  addScalarVariableCoupleableMatrixTag(tag);

  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return var->matrixTagSln(tag);
}

VariableValue &
ScalarCoupleable::coupledScalarValueOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupledScalar(var_name, comp))
    return *getDefaultValue(var_name);

  validateExecutionerType(var_name, "coupledScalarValueOld");
  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return (_sc_is_implicit) ? var->slnOld() : var->slnOlder();
}

VariableValue &
ScalarCoupleable::coupledScalarValueOlder(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupledScalar(var_name, comp))
    return *getDefaultValue(var_name);

  validateExecutionerType(var_name, "coupledScalarValueOlder");
  MooseVariableScalar * var = getScalarVar(var_name, comp);
  if (_sc_is_implicit)
    return var->slnOlder();
  else
    mooseError("Older values not available for explicit schemes");
}

VariableValue &
ScalarCoupleable::coupledScalarDot(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  validateExecutionerType(var_name, "coupledScalarDot");
  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return var->uDot();
}

VariableValue &
ScalarCoupleable::coupledScalarDotDot(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  validateExecutionerType(var_name, "coupledScalarDotDot");
  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return var->uDotDot();
}

VariableValue &
ScalarCoupleable::coupledScalarDotOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  validateExecutionerType(var_name, "coupledScalarDotOld");
  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return var->uDotOld();
}

VariableValue &
ScalarCoupleable::coupledScalarDotDotOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  validateExecutionerType(var_name, "coupledScalarDotDotOld");
  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return var->uDotDotOld();
}
VariableValue &
ScalarCoupleable::coupledScalarDotDu(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  validateExecutionerType(var_name, "coupledScalarDotDu");
  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return var->duDotDu();
}

VariableValue &
ScalarCoupleable::coupledScalarDotDotDu(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  validateExecutionerType(var_name, "coupledScalarDotDotDu");
  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return var->duDotDotDu();
}

void
ScalarCoupleable::checkVar(const std::string & var_name)
{
  auto it = _sc_coupled_vars.find(var_name);
  if (it != _sc_coupled_vars.end())
  {
    std::string cvars;
    for (auto jt : it->second)
      cvars += " " + jt->name();
    mooseError(_sc_name,
               ": Trying to couple a field variable where scalar variable is expected, '",
               var_name,
               " =",
               cvars,
               "'");
  }
  // NOTE: non-existent variables are handled in the constructor
}

MooseVariableScalar *
ScalarCoupleable::getScalarVar(const std::string & var_name, unsigned int comp)
{
  if (_coupled_scalar_vars.find(var_name) != _coupled_scalar_vars.end())
  {
    if (comp < _coupled_scalar_vars[var_name].size())
      return _coupled_scalar_vars[var_name][comp];
    else
      mooseError(_sc_name, ": Trying to get a non-existent component of variable '", var_name, "'");
  }
  else
    mooseError(_sc_name, ": Trying to get a non-existent variable '", var_name, "'");
}

void
ScalarCoupleable::validateExecutionerType(const std::string & name,
                                          const std::string & fn_name) const
{
  if (!_sc_fe_problem.isTransient())
    mooseError(_sc_name,
               ": Calling '",
               fn_name,
               "' on variable \"",
               name,
               "\" when using a \"Steady\" executioner is not allowed. This value is available "
               "only in transient simulations.");
}

unsigned int
ScalarCoupleable::coupledScalarComponents(const std::string & var_name)
{
  return _coupled_scalar_vars[var_name].size();
}
