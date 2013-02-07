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
#include "SubProblem.h"
#include "SystemBase.h"

template<>
InputParameters validParams<ScalarInitialCondition>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<VariableName>("variable", "The variable this initial condition is supposed to provide values for.");

  params.addPrivateParam<std::string>("built_by_action", "add_ic");
  return params;
}

ScalarInitialCondition::ScalarInitialCondition(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    _subproblem(*getParam<SubProblem *>("_subproblem")),
    _sys(*getParam<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getScalarVariable(_tid, parameters.get<VariableName>("variable")))
{
}

ScalarInitialCondition::~ScalarInitialCondition()
{
}

void
ScalarInitialCondition::compute(DenseVector<Number> & vals)
{
  for (_i = 0; _i < _var.order(); ++_i)
    vals(_i) = value();
}
