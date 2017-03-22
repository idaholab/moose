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

// MOOSE includes
#include "Transfer.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "MooseEnum.h"
#include "InputParameters.h"

// libMesh
#include "libmesh/system.h"

const Number Transfer::OutOfMeshValue = -999999;

template <>
InputParameters
validParams<Transfer>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  // Add the SetupInterface parameter, 'execute_on', and set it to a default of 'timestep_begin'
  params += validParams<SetupInterface>();
  params.set<MultiMooseEnum>("execute_on") = "timestep_begin";

  params.registerBase("Transfer");

  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.declareControllable("enable");
  return params;
}

Transfer::Transfer(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    Restartable(parameters, "Transfers"),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblemBase *>("_fe_problem_base")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid"))
{
}

/**
 * Small helper function for finding the system containing the variable.
 *
 * Note that this implies that variable names are unique across all systems!
 */
System *
Transfer::find_sys(EquationSystems & es, const std::string & var_name)
{
  // Find the system this variable is from
  for (unsigned int i = 0; i < es.n_systems(); i++)
    if (es.get_system(i).has_variable(var_name))
      return &es.get_system(i);

  ::mooseError("Unable to find variable " + var_name + " in any system.");

  // Unreachable
  return &es.get_system(0);
}
