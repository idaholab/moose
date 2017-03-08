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

#include "UserObject.h"
#include "SubProblem.h"
#include "Assembly.h"

// libMesh includes
#include "libmesh/sparse_matrix.h"

template <>
InputParameters
validParams<UserObject>()
{
  InputParameters params = validParams<MooseObject>();

  // Add the SetupInterface parameter, 'execute_on', and set it to a default of 'timestep_end'
  params += validParams<SetupInterface>();
  params.set<MultiMooseEnum>("execute_on") = "timestep_end";

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.declareControllable("enable");

  params.registerBase("UserObject");

  return params;
}

UserObject::UserObject(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    FunctionInterface(this),
    DistributionInterface(this),
    Restartable(parameters, "UserObjects"),
    MeshChangedInterface(parameters),
    ScalarCoupleable(this),
    _subproblem(*parameters.getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _coord_sys(_assembly.coordSystem())
{
}

UserObject::~UserObject() {}

void
UserObject::load(std::ifstream & /*stream*/)
{
}

void
UserObject::store(std::ofstream & /*stream*/)
{
}
