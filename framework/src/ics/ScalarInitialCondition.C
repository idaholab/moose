//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarInitialCondition.h"
#include "MooseVariableScalar.h"
#include "FEProblem.h"
#include "SystemBase.h"

InputParameters
ScalarInitialCondition::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += InitialConditionInterface::validParams();
  params.addParam<VariableName>(
      "variable", "The variable this initial condition is supposed to provide values for.");

  params.registerBase("ScalarInitialCondition");

  return params;
}

ScalarInitialCondition::ScalarInitialCondition(const InputParameters & parameters)
  : MooseObject(parameters),
    InitialConditionInterface(parameters),
    ScalarCoupleable(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    DependencyResolverInterface(),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _t(_fe_problem.time()),
    _var(_sys.getScalarVariable(_tid, getParam<VariableName>("variable"))),
    _assembly(
        _fe_problem.assembly(_tid, _var.kind() == Moose::VAR_SOLVER ? _var.sys().number() : 0))
{
  _supplied_vars.insert(getParam<VariableName>("variable"));

  const std::vector<MooseVariableScalar *> & coupled_vars = getCoupledMooseScalarVars();
  for (const auto & var : coupled_vars)
    _depend_vars.insert(var->name());
}

ScalarInitialCondition::~ScalarInitialCondition() {}

const std::set<std::string> &
ScalarInitialCondition::getRequestedItems()
{
  return _depend_vars;
}

const std::set<std::string> &
ScalarInitialCondition::getSuppliedItems()
{
  return _supplied_vars;
}

void
ScalarInitialCondition::compute(DenseVector<Number> & vals)
{
  for (_i = 0; _i < _var.order(); ++_i)
    vals(_i) = value();
}
