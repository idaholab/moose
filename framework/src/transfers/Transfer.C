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

#include "Transfer.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "MooseVariable.h"

template<>
InputParameters validParams<Transfer>()
{
  InputParameters params = validParams<MooseObject>();
  params.addPrivateParam<bool>("use_displaced_mesh", false);
  params.addPrivateParam<std::string>("built_by_action", "add_transfer");

  MooseEnum execute_options(SetupInterface::getExecuteOptions());
  execute_options = "timestep_begin";  // set the default
  params.addParam<MooseEnum>("execute_on", execute_options, "Set to (residual|jacobian|timestep|timestep_begin|custom) to execute only at that moment");

  return params;
}

Transfer::Transfer(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    SetupInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _execute_on(getParam<MooseEnum>("execute_on"))
{
}
