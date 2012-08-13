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

#include "Indicator.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

// libmesh includes
#include "threads.h"

template<>
InputParameters validParams<Indicator>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<FieldName>("field_name", "The name of the indicator field this indicator will add contributions to.");
  params.addParam<std::vector<SubdomainName> >("block", "The block id where this Indicator lives.");

  params.addPrivateParam<bool>("use_displaced_mesh", false);
  params.addPrivateParam<std::string>("built_by_action", "add_indicator");
  return params;
}


Indicator::Indicator(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    SetupInterface(parameters),
    Coupleable(parameters, false),
    ScalarCoupleable(parameters),
    MooseVariableInterface(parameters, false),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    MaterialPropertyInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _solution(_sys.solution()),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),

    _field_var(_sys.getVariable(_tid, parameters.get<FieldName>("field_name"))),

    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension())
{
}

void
Indicator::IndicatorSetup()
{
}
