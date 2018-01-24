//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UserObject.h"
#include "SubProblem.h"
#include "Assembly.h"

#include "libmesh/sparse_matrix.h"

template <>
InputParameters
validParams<UserObject>()
{
  InputParameters params = validParams<MooseObject>();

  // Add the SetupInterface parameter, 'execute_on', and set it to a default of 'timestep_end'
  params += validParams<SetupInterface>();
  params.set<ExecFlagEnum>("execute_on", true) = EXEC_TIMESTEP_END;

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  params.addParam<bool>("allow_duplicate_execution_on_initial",
                        false,
                        "In the case where this UserObject is depended upon by an initial "
                        "condition, allow it to be executed twice during the initial setup (once "
                        "before the IC and again after mesh adaptivity (if applicable).");

  params.declareControllable("enable");

  params.registerBase("UserObject");

  params.addParamNamesToGroup("use_displaced_mesh allow_duplicate_execution_on_initial",
                              "Advanced");
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
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _coord_sys(_assembly.coordSystem()),
    _duplicate_initial_execution(getParam<bool>("allow_duplicate_execution_on_initial"))
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
