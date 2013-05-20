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

#include "InitialCondition.h"
#include "FEProblem.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "MooseVariable.h"

template<>
InputParameters validParams<InitialCondition>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<VariableName>("variable", "The variable this InitialCondtion is supposed to provide values for.");
  params.addParam<std::vector<SubdomainName> >("block", "The list of ids or names of the blocks (subdomain) that this initial condition will be applied to");

  params.addPrivateParam<std::string>("built_by_action", "add_ic");
  return params;
}

InitialCondition::InitialCondition(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    _fe_problem(*getParam<FEProblem *>("_fe_problem")),
    _sys(*getParam<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_fe_problem.assembly(_tid)),
    _t(_fe_problem.time()),
    _coord_sys(_assembly.coordSystem()),
    _var(_sys.getVariable(_tid, parameters.get<VariableName>("variable"))),

    _current_elem(_var.currentElem())
{
}

InitialCondition::~InitialCondition()
{
}

