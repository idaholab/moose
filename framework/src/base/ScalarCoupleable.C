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
#include "FEProblem.h"

ScalarCoupleable::ScalarCoupleable(const MooseObject * moose_object)
  : _sc_parameters(moose_object->parameters()),
    _sc_fe_problem(*_sc_parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sc_is_implicit(_sc_parameters.have_parameter<bool>("implicit")
                        ? _sc_parameters.get<bool>("implicit")
                        : true),
    _coupleable_params(_sc_parameters)
{
  SubProblem & problem = *_sc_parameters.get<SubProblem *>("_subproblem");

  THREAD_ID tid =
      _sc_parameters.have_parameter<THREAD_ID>("_tid") ? _sc_parameters.get<THREAD_ID>("_tid") : 0;

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
  for (auto & it : _default_value)
  {
    it.second->release();
    delete it.second;
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
  std::map<std::string, std::vector<MooseVariableScalar *>>::iterator it =
      _coupled_scalar_vars.find(var_name);
  if (it != _coupled_scalar_vars.end())
    return (i < it->second.size());
  else
  {
    // Make sure the user originally requested this value in the InputParameter syntax
    if (!_coupleable_params.hasCoupledValue(var_name))
      mooseError("The coupled scalar variable \"",
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
  return getScalarVar(var_name, comp)->number();
}

Order
ScalarCoupleable::coupledScalarOrder(const std::string & var_name, unsigned int comp)
{
  if (!isCoupledScalar(var_name, comp))
    return _sc_fe_problem.getMaxScalarOrder();

  return getScalarVar(var_name, comp)->order();
}

VariableValue *
ScalarCoupleable::getDefaultValue(const std::string & var_name)
{
  std::map<std::string, VariableValue *>::iterator default_value_it = _default_value.find(var_name);
  if (default_value_it == _default_value.end())
  {
    VariableValue * value = new VariableValue(_sc_fe_problem.getMaxScalarOrder(),
                                              _coupleable_params.defaultCoupledValue(var_name));
    default_value_it = _default_value.insert(std::make_pair(var_name, value)).first;
  }

  return default_value_it->second;
}

VariableValue &
ScalarCoupleable::coupledScalarValue(const std::string & var_name, unsigned int comp)
{
  if (!isCoupledScalar(var_name, comp))
    return *getDefaultValue(var_name);

  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return (_sc_is_implicit) ? var->sln() : var->slnOld();
}

VariableValue &
ScalarCoupleable::coupledScalarValueOld(const std::string & var_name, unsigned int comp)
{
  if (!isCoupledScalar(var_name, comp))
    return *getDefaultValue(var_name);

  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return (_sc_is_implicit) ? var->slnOld() : var->slnOlder();
}

VariableValue &
ScalarCoupleable::coupledScalarDot(const std::string & var_name, unsigned int comp)
{
  MooseVariableScalar * var = getScalarVar(var_name, comp);
  return var->uDot();
}

VariableValue &
ScalarCoupleable::coupledScalarDotDu(const std::string & var_name, unsigned int comp)
{
  MooseVariableScalar * var = getScalarVar(var_name, comp);
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
