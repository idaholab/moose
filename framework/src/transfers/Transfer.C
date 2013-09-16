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
  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  params.addPrivateParam<std::string>("built_by_action", "add_transfer");

  // Add the SetupInterface parameter, 'execute_on', and set it to a default of 'timestep'
  params += validParams<SetupInterface>();
  params.set<MooseEnum>("execute_on") = "timestep_begin";

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

/**
 * Small helper function for finding the system containing the variable.
 *
 * Note that this implies that variable names are unique across all systems!
 */
System *
Transfer::find_sys(EquationSystems & es, std::string & var_name)
{
  System * sys = NULL;

  // Find the system this variable is from
  for(unsigned int i=0; i<es.n_systems(); i++)
  {
    if(es.get_system(i).has_variable(var_name))
    {
      sys = &es.get_system(i);
      break;
    }
  }

  mooseAssert(sys, "Unable to find variable " + var_name);

  return sys;
}
