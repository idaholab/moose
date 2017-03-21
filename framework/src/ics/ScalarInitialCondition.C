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

#include "ScalarInitialCondition.h"
#include "MooseVariableScalar.h"
#include "FEProblem.h"
#include "SystemBase.h"

template <>
InputParameters
validParams<ScalarInitialCondition>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<VariableName>(
      "variable", "The variable this initial condition is supposed to provide values for.");

  params.registerBase("ScalarInitialCondition");

  return params;
}

ScalarInitialCondition::ScalarInitialCondition(const InputParameters & parameters)
  : MooseObject(parameters),
    ScalarCoupleable(this),
    FunctionInterface(this),
    DependencyResolverInterface(),
    _fe_problem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*parameters.getCheckedPointerParam<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_fe_problem.assembly(_tid)),
    _t(_fe_problem.time()),
    _var(_sys.getScalarVariable(_tid, getParam<VariableName>("variable")))
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
