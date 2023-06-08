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
  : _sc_fe_problem(
        *moose_object->parameters().getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sc_tid(moose_object->parameters().isParamValid("_tid")
                ? moose_object->parameters().get<THREAD_ID>("_tid")
                : 0),
    _real_zero(_sc_fe_problem._real_zero[_sc_tid]),
    _scalar_zero(_sc_fe_problem._scalar_zero[_sc_tid]),
    _point_zero(_sc_fe_problem._point_zero[_sc_tid]),
    _sc_parameters(moose_object->parameters()),
    _sc_name(_sc_parameters.get<std::string>("_object_name")),
    _sc_is_implicit(_sc_parameters.have_parameter<bool>("implicit")
                        ? _sc_parameters.get<bool>("implicit")
                        : true)
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

bool
ScalarCoupleable::isCoupledScalar(const std::string & var_name_in, unsigned int i) const
{
  const auto var_name = _sc_parameters.checkForRename(var_name_in);

  auto it = _coupled_scalar_vars.find(var_name);
  if (it != _coupled_scalar_vars.end())
    return (i < it->second.size());
  else
  {
    // Make sure the user originally requested this value in the InputParameter syntax
    if (!_sc_parameters.hasCoupledValue(var_name))
      mooseError(_sc_name,
                 ": The coupled scalar variable \"",
                 var_name,
                 "\" was never added to this object's "
                 "InputParameters, please double-check "
                 "your spelling");

    return false;
  }
}

unsigned int
ScalarCoupleable::coupledScalar(const std::string & var_name, unsigned int comp) const
{
  checkVar(var_name);
  return getScalarVar(var_name, comp)->number();
}

Order
ScalarCoupleable::coupledScalarOrder(const std::string & var_name, unsigned int comp) const
{
  checkVar(var_name);
  if (!isCoupledScalar(var_name, comp))
    return _sc_fe_problem.getMaxScalarOrder();

  return getScalarVar(var_name, comp)->order();
}

const VariableValue *
ScalarCoupleable::getDefaultValue(const std::string & var_name) const
{
  auto default_value_it = _default_value.find(var_name);
  if (default_value_it == _default_value.end())
  {
    auto value = std::make_unique<VariableValue>(_sc_fe_problem.getMaxScalarOrder(),
                                                 _sc_parameters.defaultCoupledValue(var_name));
    default_value_it = _default_value.insert(std::make_pair(var_name, std::move(value))).first;
  }

  return default_value_it->second.get();
}

const VariableValue &
ScalarCoupleable::coupledScalarValue(const std::string & var_name, unsigned int comp) const
{
  checkVar(var_name);
  if (!isCoupledScalar(var_name, comp))
    return *getDefaultValue(var_name);

  auto var = getScalarVar(var_name, comp);
  return _sc_is_implicit ? var->sln() : var->slnOld();
}

const ADVariableValue &
ScalarCoupleable::adCoupledScalarValue(const std::string & var_name, unsigned int comp) const
{
  checkVar(var_name);
  if (!isCoupledScalar(var_name, comp))
    return *getADDefaultValue(var_name);

  auto var = getScalarVar(var_name, comp);

  if (_sc_is_implicit)
    return var->adSln();
  else
    mooseError("adCoupledValue for non-implicit calculations is not currently supported. Use "
               "coupledValue instead for non-implicit");
}

template <>
const GenericVariableValue<false> &
ScalarCoupleable::coupledGenericScalarValue<false>(const std::string & var_name,
                                                   unsigned int comp) const
{
  return coupledScalarValue(var_name, comp);
}

template <>
const GenericVariableValue<true> &
ScalarCoupleable::coupledGenericScalarValue<true>(const std::string & var_name,
                                                  unsigned int comp) const
{
  return adCoupledScalarValue(var_name, comp);
}

const ADVariableValue *
ScalarCoupleable::getADDefaultValue(const std::string & var_name) const
{
  auto default_value_it = _dual_default_value.find(var_name);
  if (default_value_it == _dual_default_value.end())
  {
    auto value = std::make_unique<ADVariableValue>(_sc_fe_problem.getMaxScalarOrder(),
                                                   _sc_parameters.defaultCoupledValue(var_name));
    default_value_it = _dual_default_value.insert(std::make_pair(var_name, std::move(value))).first;
  }

  return default_value_it->second.get();
}

const VariableValue &
ScalarCoupleable::coupledVectorTagScalarValue(const std::string & var_name,
                                              TagID tag,
                                              unsigned int comp) const
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

  _sc_coupleable_vector_tags.insert(tag);

  return getScalarVar(var_name, comp)->vectorTagSln(tag);
}

const VariableValue &
ScalarCoupleable::coupledMatrixTagScalarValue(const std::string & var_name,
                                              TagID tag,
                                              unsigned int comp) const
{
  checkVar(var_name);
  if (!isCoupledScalar(var_name, comp))
    return *getDefaultValue(var_name);

  _sc_coupleable_matrix_tags.insert(tag);

  return getScalarVar(var_name, comp)->matrixTagSln(tag);
}

const VariableValue &
ScalarCoupleable::coupledScalarValueOld(const std::string & var_name, unsigned int comp) const
{
  checkVar(var_name);
  if (!isCoupledScalar(var_name, comp))
    return *getDefaultValue(var_name);

  validateExecutionerType(var_name, "coupledScalarValueOld");
  auto var = getScalarVar(var_name, comp);
  return _sc_is_implicit ? var->slnOld() : var->slnOlder();
}

const VariableValue &
ScalarCoupleable::coupledScalarValueOlder(const std::string & var_name, unsigned int comp) const
{
  checkVar(var_name);
  if (!isCoupledScalar(var_name, comp))
    return *getDefaultValue(var_name);

  validateExecutionerType(var_name, "coupledScalarValueOlder");
  auto var = getScalarVar(var_name, comp);
  if (_sc_is_implicit)
    return var->slnOlder();
  else
    mooseError("Older values not available for explicit schemes");
}

const VariableValue &
ScalarCoupleable::coupledScalarDot(const std::string & var_name, unsigned int comp) const
{
  checkVar(var_name);
  validateExecutionerType(var_name, "coupledScalarDot");
  return getScalarVar(var_name, comp)->uDot();
}

const VariableValue &
ScalarCoupleable::coupledScalarDotDot(const std::string & var_name, unsigned int comp) const
{
  checkVar(var_name);
  validateExecutionerType(var_name, "coupledScalarDotDot");
  return getScalarVar(var_name, comp)->uDotDot();
}

const VariableValue &
ScalarCoupleable::coupledScalarDotOld(const std::string & var_name, unsigned int comp) const
{
  checkVar(var_name);
  validateExecutionerType(var_name, "coupledScalarDotOld");
  return getScalarVar(var_name, comp)->uDotOld();
}

const VariableValue &
ScalarCoupleable::coupledScalarDotDotOld(const std::string & var_name, unsigned int comp) const
{
  checkVar(var_name);
  validateExecutionerType(var_name, "coupledScalarDotDotOld");
  return getScalarVar(var_name, comp)->uDotDotOld();
}
const VariableValue &
ScalarCoupleable::coupledScalarDotDu(const std::string & var_name, unsigned int comp) const
{
  checkVar(var_name);
  validateExecutionerType(var_name, "coupledScalarDotDu");
  return getScalarVar(var_name, comp)->duDotDu();
}

const VariableValue &
ScalarCoupleable::coupledScalarDotDotDu(const std::string & var_name, unsigned int comp) const
{
  checkVar(var_name);
  validateExecutionerType(var_name, "coupledScalarDotDotDu");
  return getScalarVar(var_name, comp)->duDotDotDu();
}

void
ScalarCoupleable::checkVar(const std::string & var_name_in) const
{
  const auto var_name = _sc_parameters.checkForRename(var_name_in);

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

const MooseVariableScalar *
ScalarCoupleable::getScalarVar(const std::string & var_name_in, unsigned int comp) const
{
  const auto var_name = _sc_parameters.checkForRename(var_name_in);

  const auto it = _coupled_scalar_vars.find(var_name);
  if (it != _coupled_scalar_vars.end())
  {
    const auto & entry = it->second;
    if (comp < entry.size())
      return entry[comp];
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
ScalarCoupleable::coupledScalarComponents(const std::string & var_name_in) const
{
  const auto var_name = _sc_parameters.checkForRename(var_name_in);

  const auto it = _coupled_scalar_vars.find(var_name);
  if (it != _coupled_scalar_vars.end())
    return it->second.size();

  mooseError(_sc_name, ": Trying to get a non-existent variable '", var_name, "'");
}
